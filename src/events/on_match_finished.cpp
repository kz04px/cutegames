#include <elo.hpp>
#include <iomanip>
#include <iostream>
#include <termcolor/termcolor.hpp>
#include "on_events.hpp"
#include "print.hpp"

[[nodiscard]] constexpr auto get_score(const int w, const int l, const int d) -> float {
    return static_cast<float>(2 * w + d) / (2 * (w + d + l));
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

auto on_match_finished(const std::shared_ptr<libevents::Event> &,
                       bool &quit,
                       const MatchSettings &settings,
                       MatchStatistics &stats,
                       const std::vector<EngineStatistics> &engine_stats,
                       Store<UGIEngine> &engine_store) noexcept -> void {
    quit = true;

    stats.num_engine_unloads += engine_store.size();
    engine_store.clear();

    if (stats.num_games_finished >= settings.update_frequency &&
        stats.num_games_finished % settings.update_frequency != 0) {
        print_results(settings.engine_settings, engine_stats, true);
    }
}
