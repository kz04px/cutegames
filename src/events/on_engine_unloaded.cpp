#include <iostream>
#include "colour.hpp"
#include "events.hpp"
#include "on_events.hpp"
#include "print.hpp"

auto on_engine_unloaded(const std::shared_ptr<libevents::Event> &event,
                        const MatchSettings &settings,
                        MatchStatistics &stats) noexcept -> void {
    const auto e = std::static_pointer_cast<EngineDestroyed>(event);

    stats.num_engine_unloads++;

    if (settings.debug) {
        std::scoped_lock<std::mutex> lock(print_mutex);
        std::cout << termcolor::blue;
        std::cout << "[debug] ";
        std::cout << termcolor::reset;
        std::cout << "Unload engine " << e->engine_id << "\n";
    }
}
