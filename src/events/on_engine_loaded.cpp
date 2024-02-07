#include <iostream>
#include "colour.hpp"
#include "events.hpp"
#include "on_events.hpp"
#include "print.hpp"

auto on_engine_loaded(const std::shared_ptr<libevents::Event> &event,
                      const MatchSettings &settings,
                      MatchStatistics &stats) noexcept -> void {
    const auto e = std::static_pointer_cast<EngineCreated>(event);

    stats.num_engine_loads++;

    if (settings.verbose) {
        std::scoped_lock<std::mutex> lock(print_mutex);
        std::cout << termcolor::blue;
        std::cout << "[verbose] ";
        std::cout << termcolor::reset;
        std::cout << "Load engine " << e->engine_id << "\n";
    }
}
