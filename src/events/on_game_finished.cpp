#include <cassert>
#include <elo.hpp>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <termcolor/termcolor.hpp>
#include "on_events.hpp"
#include "print.hpp"

[[nodiscard]] constexpr auto get_score(const int w, const int l, const int d) -> float {
    return static_cast<float>(2 * w + d) / (2 * (w + d + l));
}

[[nodiscard]] constexpr auto should_update(const int num, const int frequency) noexcept -> bool {
    assert(frequency > 0);
    return num % frequency == 0 || num < frequency;
}

void print_results(const auto &engine_settings, const auto &engine_stats, const bool print_elo) {
    std::scoped_lock<std::mutex> lock(print_mutex);

    if (engine_settings.size() == 2) {
        const auto w = engine_stats[0].win;
        const auto l = engine_stats[0].lose;
        const auto d = engine_stats[0].draw;
        const auto score = get_score(w, l, d);

        std::cout << std::fixed << std::setprecision(3) << "Score of " << engine_settings[0].name << " vs "
                  << engine_settings[1].name << ": " << w << " - " << l << " - " << d << " [" << score << "] "
                  << engine_stats[0].played << "\n";
        if (print_elo) {
            const auto elo = get_elo(w, l, d);
            const auto err = get_err(w, l, d);
            std::cout << std::fixed << std::setprecision(2) << elo << " +/- " << err << "\n";
            std::cout << "\n";
        }
    } else {
        std::cout << "Name  Wins  Losses  Draws\n";
        for (std::size_t i = 0; i < engine_settings.size(); ++i) {
            std::cout << engine_settings[i].name;
            std::cout << "  " << engine_stats[i].win;
            std::cout << "  " << engine_stats[i].lose;
            std::cout << "  " << engine_stats[i].draw;
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

auto on_game_finished(const std::shared_ptr<libevents::Event> &event,
                      const MatchSettings &settings,
                      MatchStatistics &stats,
                      std::vector<EngineStatistics> &engine_stats,
                      libevents::Dispatcher &dispatcher) noexcept -> void {
    const auto e = std::static_pointer_cast<GameFinished>(event);

    stats.num_games_finished++;
    engine_stats.at(e->engine1_id).played++;
    engine_stats.at(e->engine2_id).played++;

    switch (e->result) {
        case GameResult::Player1Win:
            stats.num_p1_wins++;
            engine_stats.at(e->engine1_id).win++;
            engine_stats.at(e->engine2_id).lose++;
            break;
        case GameResult::Player2Win:
            stats.num_p2_wins++;
            engine_stats.at(e->engine1_id).lose++;
            engine_stats.at(e->engine2_id).win++;
            break;
        case GameResult::Draw:
            stats.num_draws++;
            engine_stats.at(e->engine1_id).draw++;
            engine_stats.at(e->engine2_id).draw++;
            break;
        case GameResult::None:
            break;
        default:
            break;
    }

    if (settings.verbose) {
        std::scoped_lock<std::mutex> lock(print_mutex);
        std::cout << "Finished game " << stats.num_games_finished << " of " << settings.num_games << "\n";
    }

    if (should_update(stats.num_games_finished, settings.update_frequency)) {
        const auto print_elo = engine_stats.size() == 2 && stats.num_games_finished >= settings.update_frequency;
        print_results(settings.engine_settings, engine_stats, print_elo);
    }

    if (stats.num_games_finished >= stats.num_games_total) {
        dispatcher.post_event(std::make_shared<MatchFinished>());
    }
}
