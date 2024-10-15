#include <CLI/CLI.hpp>
#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
// Games
#include "games/game.hpp"
// Events
#include "engine/engine.hpp"
#include "events/events.hpp"
#include "events/on_events.hpp"
// Match
#include "match/openings.hpp"
#include "match/play.hpp"
#include "match/settings.hpp"
// Tournaments
#include "tournament/gauntlet.hpp"
#include "tournament/generator.hpp"
#include "tournament/roundrobin.hpp"
// Engines
#include "engine/engine_uai.hpp"
#include "engine/engine_uci.hpp"
#include "engine/engine_ugi.hpp"
// Stuff
#include "cutegames.hpp"
#include "store.hpp"
#include "tournament/types.hpp"

[[nodiscard]] auto make_engine(const GameType game_type, const EngineSettings &settings, const bool debug = false)
    -> std::shared_ptr<Engine> {
    auto make_engine = [&game_type, &settings]() -> std::shared_ptr<Engine> {
        switch (game_type) {
            case GameType::Generic:
                return std::make_shared<UGIEngine>(settings.id, settings.path, settings.parameters);
            case GameType::Ataxx:
                return std::make_shared<UAIEngine>(settings.id, settings.path, settings.parameters);
            case GameType::Chess:
                return std::make_shared<UCIEngine>(settings.id, settings.path, settings.parameters);
            default:
                throw std::invalid_argument("Unrecognised game type");
        }
    };

    auto make_engine_debug = [&game_type, &settings]() -> std::shared_ptr<Engine> {
        const auto debug_recv = [](const std::string_view &msg) {
            std::cout << "<recv:" << std::this_thread::get_id() << "> " << msg << "\n";
        };

        const auto debug_send = [](const std::string_view &msg) {
            std::cout << "<send:" << std::this_thread::get_id() << "> " << msg << "\n";
        };

        switch (game_type) {
            case GameType::Generic:
                return std::make_shared<UGIEngine>(
                    settings.id, settings.path, settings.parameters, debug_recv, debug_send);
            case GameType::Ataxx:
                return std::make_shared<UAIEngine>(
                    settings.id, settings.path, settings.parameters, debug_recv, debug_send);
            case GameType::Chess:
                return std::make_shared<UCIEngine>(
                    settings.id, settings.path, settings.parameters, debug_recv, debug_send);
            default:
                throw std::invalid_argument("Unrecognised game type");
        }
    };

    auto engine = debug ? make_engine_debug() : make_engine();

    engine->init();

    // Set options
    for (const auto &[name, value] : settings.options) {
        engine->set_option(name, value);
    }

    engine->is_ready();

    return engine;
}

auto print_engine_settings(const std::vector<EngineSettings> &engine_settings) noexcept -> void {
    std::cout << "Engine Data:\n";
    for (const auto &data : engine_settings) {
        std::cout << "- " << data.id;
        std::cout << " " << data.name;
        switch (data.protocol) {
            case EngineProtocol::UGI:
                std::cout << " UGI";
                break;
            case EngineProtocol::UAI:
                std::cout << " UAI";
                break;
            case EngineProtocol::UCI:
                std::cout << " UCI";
                break;
        }
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

[[nodiscard]] auto make_generator(const TournamentType type,
                                  const std::size_t num_engines,
                                  const int num_games,
                                  const std::size_t num_openings,
                                  const bool repeat) -> std::shared_ptr<TournamentGenerator> {
    switch (type) {
        case TournamentType::RoundRobin:
            return std::make_shared<RoundRobinGenerator>(num_engines, num_games, num_openings, repeat);
        case TournamentType::Gauntlet:
            return std::make_shared<GauntletGenerator>(num_engines, num_games, num_openings, repeat);
        default:
            throw std::invalid_argument("Unknown tournament type");
    }
}

auto main(const int argc, const char *const *const argv) noexcept -> int {
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
        settings.engine_store_size = *override_store;
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
    dispatcher.register_event_listener(EventID::zMatchFinished, [&quit](const auto &event) {
        on_match_finished(event, quit);
    });

    const auto t0 = std::chrono::steady_clock::now();

    auto engine_data = std::vector<EngineStatistics>(settings.engine_settings.size());
    std::vector<std::thread> workers;
    std::mutex mtx;
    auto generator = make_generator(settings.tournament_type,
                                    settings.engine_settings.size(),
                                    settings.num_games,
                                    openings.size(),
                                    settings.repeat);

    stats.num_games_total = generator->expected();

    for (std::size_t i = 0; i < settings.num_threads; ++i) {
        workers.emplace_back([&]() {
            auto engine_store = Store<Engine>(settings.engine_store_size);

            while (!quit) {
                // Get work
                const auto info = [&generator, &mtx]() -> std::optional<GameInfo> {
                    std::scoped_lock lock(mtx);
                    if (generator->is_finished()) {
                        return {};
                    }
                    return generator->next();
                }();

                // Finish if no work left
                if (!info) {
                    break;
                }

                assert(info->idx_player1 != info->idx_player2);
                assert(info->idx_player1 < engine_data.size());
                assert(info->idx_player2 < engine_data.size());
                assert(info->idx_opening < openings.size());

                // Try get engines from store
                auto engine1 = engine_store.get([id = info->idx_player1](const auto &obj) noexcept -> bool {
                    return id == obj->get_id();
                });
                auto engine2 = engine_store.get([id = info->idx_player2](const auto &obj) noexcept -> bool {
                    return id == obj->get_id();
                });

                // Create engine instance if not returned from store
                if (!engine1) {
                    engine1 =
                        make_engine(settings.game_type, settings.engine_settings[info->idx_player1], settings.debug);
                    dispatcher.post_event(
                        std::make_shared<EngineCreated>(info->idx_player1,
                                                        settings.engine_settings[info->idx_player1].name,
                                                        settings.engine_settings[info->idx_player1].path));
                }
                if (!engine2) {
                    engine2 =
                        make_engine(settings.game_type, settings.engine_settings[info->idx_player2], settings.debug);
                    dispatcher.post_event(
                        std::make_shared<EngineCreated>(info->idx_player2,
                                                        settings.engine_settings[info->idx_player2].name,
                                                        settings.engine_settings[info->idx_player2].path));
                }

                dispatcher.post_event(std::make_shared<GameStarted>(
                    info->id, openings.at(info->idx_opening), (*engine1)->get_id(), (*engine2)->get_id()));

                const auto gg = play_game(settings.game_type,
                                          settings.timecontrol,
                                          settings.adjudication,
                                          settings.protocol,
                                          openings.at(info->idx_opening),
                                          *engine1,
                                          *engine2);

                dispatcher.post_event(std::make_shared<GameFinished>(
                    info->id, (*engine1)->get_id(), (*engine2)->get_id(), gg.result, gg.reason, gg.game));

                // Return the engines now we're done with them
                const auto released1 = engine_store.release(*engine1);
                const auto released2 = engine_store.release(*engine2);

                if (released1) {
                    dispatcher.post_event(std::make_shared<EngineDestroyed>(99, "", ""));
                }
                if (released2) {
                    dispatcher.post_event(std::make_shared<EngineDestroyed>(99, "", ""));
                }
            }

            // Clear the store
            while (!engine_store.empty()) {
                engine_store.remove_oldest();
                dispatcher.post_event(std::make_shared<EngineDestroyed>(99, "", ""));
            }
        });
    }

    // Event listener
    while (!quit) {
        dispatcher.wait();
        dispatcher.send_all();
    }

    while (!dispatcher.empty()) {
        dispatcher.send_all();
    }

    for (auto &thread : workers) {
        if (thread.joinable()) {
            thread.join();
        }
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
        const auto games_per_ms = static_cast<float>(stats.num_games_finished) / static_cast<float>(dt.count());
        const auto games_per_s = games_per_ms * 1'000;
        const auto games_per_min = games_per_ms * 60'000;
        std::cout << "Games/min: " << games_per_min << "\n";
        std::cout << "Games/sec: " << games_per_s << "\n";
        std::cout << "Games/ms: " << games_per_ms << "\n";
        std::cout << "ms/game: " << dt.count() / stats.num_games_finished << "\n";
    }

    return 0;
}
