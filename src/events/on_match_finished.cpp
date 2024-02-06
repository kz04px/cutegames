#include "on_events.hpp"

auto on_match_finished(const std::shared_ptr<libevents::Event> &, bool &quit) noexcept -> void {
    quit = true;
}
