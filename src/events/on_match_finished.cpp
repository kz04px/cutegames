#include <iostream>
#include "on_events.hpp"

auto on_match_finished(const std::shared_ptr<libevents::Event> &,
                       bool &quit,
                       Store<UGIEngine> &engine_store,
                       libevents::Dispatcher &dispatcher) noexcept -> void {
    quit = true;

    while (!engine_store.empty()) {
        engine_store.remove_oldest();
        dispatcher.post_event(std::make_shared<EngineDestroyed>(99, "", ""));
    }
}
