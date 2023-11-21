#include <iostream>
#include <string_view>
#include <thread>
#include "engine_ugi.hpp"

[[nodiscard]] auto make_engine(const EngineSettings &settings, const bool debug = false) -> std::shared_ptr<UGIEngine> {
    if (debug) {
        const auto debug_recv = [](const std::string_view &msg) {
            std::cout << "<recv:" << std::this_thread::get_id() << "> " << msg << "\n";
        };

        const auto debug_send = [](const std::string_view &msg) {
            std::cout << "<send:" << std::this_thread::get_id() << "> " << msg << "\n";
        };

        return std::make_shared<UGIEngine>(settings.id, settings.path, settings.parameters, debug_recv, debug_send);
    } else {
        return std::make_shared<UGIEngine>(settings.id, settings.path, settings.parameters);
    }
}
