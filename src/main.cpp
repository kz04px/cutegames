#include <CLI/CLI.hpp>
#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
// gg
#include <games/game.hpp>
#include <threadpool.hpp>
// Stuff
#include "cutegames.hpp"
#include "events/events.hpp"
#include "events/on_events.hpp"
#include "match.hpp"
#include "openings.hpp"
#include "play.hpp"
#include "settings.hpp"
#include "store.hpp"
// Tournaments
#include "tournament/gauntlet.hpp"
#include "tournament/generator.hpp"
#include "tournament/roundrobin.hpp"

auto print_engine_settings(const std::vector<EngineSettings> &engine_settings) noexcept -> void {
    std::cout << "Engine Data:\n";
    for (const auto &data : engine_settings) {
        std::cout << "- " << data.id;
        std::cout << " " << data.name;
        std::cout << " " << data.path;
        std::cout << " " << data.parameters;
        for (const auto &[key, val] : data.options) {
            std::cout << " {" << key << ":" << val << "}";
        }
        std::cout << "\n";
    }
}

auto print_statistics(const MatchStatistics &stats) noexcept -> void {
    std::cout << "Statistics:\n";
    std::cout << "Engines loaded: " << stats.num_engine_loads << "\n";
    std::cout << "Engines unloaded: " << stats.num_engine_unloads << "\n";
    std::cout << "Games finished: " << stats.num_games_finished << "\n";
    std::cout << "Player 1 Score: +" << stats.num_p1_wins << "-" << stats.num_p2_wins << "=" << stats.num_draws << "\n";
}

auto print_about() noexcept -> void {
    std::cout << "Cute Games v" << version_major << "." << version_minor;
#ifndef NDEBUG
    std::cout << " (debug)";
#endif
    std::cout << "\n";
    std::cout << "Authored by kz04px\n";
    std::cout << "https://github.com/kz04px/cutegames\n";
}

auto main(const int argc, const char **argv) noexcept -> int {
    CLI::App app;

    auto settings_path = std::string();
    std::optional<int> override_threads;
    std::optional<int> override_num_games;
    std::optional<int> override_store;
    std::optional<bool> override_debug;
    std::optional<bool> override_verbose;

    app.add_option("--settings", settings_path, "Path to settings json")->required();
    app.add_option("--threads", override_threads, "Number of threads to use")->check(CLI::PositiveNumber);
    app.add_option("--games", override_num_games, "Number of games to play per matchup")->check(CLI::PositiveNumber);
    app.add_option("--store", override_store, "Size of the engine store");
    app.add_flag("--debug", override_debug, "Enable debug");
    app.add_flag("--verbose", override_verbose, "Verbose output");
    app.set_version_flag("--version",
                         "Cute Games v" + std::to_string(version_major) + "." + std::to_string(version_minor));

    CLI11_PARSE(app, argc, argv);

    print_about();
    std::cout << "\n";

    auto settings = get_settings(settings_path);

    if (override_threads) {
        settings.num_threads = *override_threads;
    }

    if (override_num_games) {
        settings.num_games = *override_num_games;
    }

    if (override_store) {
        settings.engine_store_size = *override_store * settings.num_threads;
    }

    if (override_debug) {
        settings.debug = *override_debug;
    }

    if (override_verbose) {
        settings.verbose = *override_verbose;
    }

    // Disable buffering for stdin & stdout
    std::setbuf(stdin, nullptr);
    std::setbuf(stdout, nullptr);

    auto quit = false;
    const auto openings = get_openings(settings.openings_path, settings.shuffle_openings);
    auto engine_store = Store<Engine>(settings.engine_store_size);
    auto dispatcher = libevents::Dispatcher();
    auto engine_statistics = std::vector<EngineStatistics>(settings.engine_settings.size());
    MatchStatistics stats;

    if (openings.empty()) {
        std::cerr << "No opening positions found\n";
        return 1;
    }

    print_settings(settings);
    std::cout << "\n";
    print_engine_settings(settings.engine_settings);
    std::cout << "\n";
    std::cout << "Opening positions: " << openings.size() << "\n";
    std::cout << "\n";

    // Register event handlers
    dispatcher.register_event_listener(EventID::zGameStarted, [&settings](const auto &event) {
        on_game_started(event, settings);
    });
    dispatcher.register_event_listener(EventID::zGameFinished,
                                       [&settings, &stats, &engine_statistics, &dispatcher](const auto &event) {
                                           on_game_finished(event, settings, stats, engine_statistics, dispatcher);
                                       });
    dispatcher.register_event_listener(EventID::zEngineLoaded, [&settings, &stats](const auto &event) {
        on_engine_loaded(event, settings, stats);
    });
    dispatcher.register_event_listener(EventID::zEngineUnloaded, [&settings, &stats](const auto &event) {
        on_engine_unloaded(event, settings, stats);
    });
    dispatcher.register_event_listener(EventID::zMatchFinished, [&quit, &engine_store, &dispatcher](const auto &event) {
        on_match_finished(event, quit, engine_store, dispatcher);
    });

    auto engine_data = std::vector<EngineStatistics>(settings.engine_settings.size());

    const auto t0 = std::chrono::steady_clock::now();

    std::shared_ptr<TournamentGenerator> generator;

    switch (settings.tournament_type) {
        case TournamentType::RoundRobin:
            generator = std::make_shared<RoundRobinGenerator>(
                settings.engine_settings.size(), settings.num_games, openings.size(), settings.repeat);
            break;
        case TournamentType::Gauntlet:
            generator = std::make_shared<GauntletGenerator>(
                settings.engine_settings.size(), settings.num_games, openings.size(), settings.repeat);
            break;
        default:
            std::cerr << "Unknown tournament type" << std::endl;
            return -1;
    }

    // Create work to do
    ThreadPool tp{settings.num_threads};
    while (!generator->is_finished()) {
        const auto info = generator->next();

        assert(info.idx_player1 != info.idx_player2);
        assert(info.idx_player1 < engine_data.size());
        assert(info.idx_player2 < engine_data.size());
        assert(info.idx_opening < openings.size());

        tp.add_job([&settings, &dispatcher, &engine_store, info, &openings]() {
            play_game(info.id,
                      openings.at(info.idx_opening),
                      info.idx_player1,
                      info.idx_player2,
                      settings,
                      dispatcher,
                      engine_store);
        });
        stats.num_games_total++;
    }

    // Event listener
    while (!quit) {
        dispatcher.wait();
        dispatcher.send_all();
    }

    tp.clear();

    while (!dispatcher.empty()) {
        dispatcher.send_all();
    }

    const auto t1 = std::chrono::steady_clock::now();
    const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    std::chrono::hh_mm_ss<std::chrono::milliseconds> tod{dt};

    std::cout << "\n";
    print_statistics(stats);
    std::cout << "\n";
    std::cout << "Time taken:";
    if (tod.hours().count() > 0) {
        std::cout << " " << tod.hours().count() << "h";
    }
    std::cout << " " << tod.minutes().count() << "m";
    std::cout << " " << tod.seconds().count() << "s";
    std::cout << "\n";
    if (dt.count() > 0 && stats.num_games_finished > 0) {
        const auto games_per_ms = static_cast<float>(stats.num_games_finished) / dt.count();
        const auto games_per_s = games_per_ms * 1'000;
        const auto games_per_min = games_per_ms * 60'000;
        std::cout << "Games/min: " << games_per_min << "\n";
        std::cout << "Games/sec: " << games_per_s << "\n";
        std::cout << "Games/ms: " << games_per_ms << "\n";
        std::cout << "ms/game: " << dt.count() / stats.num_games_finished << "\n";
    }

    return 0;
}
