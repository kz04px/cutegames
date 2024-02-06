#include <iostream>
#include "events.hpp"
#include "on_events.hpp"
#include "print.hpp"

auto on_game_started(const std::shared_ptr<libevents::Event> &event, const MatchSettings &settings) noexcept -> void {
    const auto e = std::static_pointer_cast<GameStarted>(event);

    if (settings.verbose) {
        std::scoped_lock<std::mutex> lock(print_mutex);
        const auto &engine1_settings = settings.engine_settings[e->engine1_id];
        const auto &engine2_settings = settings.engine_settings[e->engine2_id];
        std::cout << "Starting game " << e->game_num << ": " << engine1_settings.name << " vs " << engine2_settings.name
                  << "\n";
    }
}
