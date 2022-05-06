#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <chrono>
#include <cstdlib>
#include <string>
#include <vector>
#include "engine/engine.hpp"

struct MatchSettings {
    std::size_t num_threads = 1;
    std::chrono::milliseconds timeoutbuffer = std::chrono::milliseconds(10);
    int num_games = 1;
    int engine_store_size = 2;
    int update_frequency = 10;
    int maxfullmoves = 0;
    std::string openings_path;
    std::vector<EngineSettings> engine_settings;
    SearchSettings timecontrol;
    bool shuffle_openings = false;
    bool repeat = true;
    bool debug = false;
    bool recover = true;
    bool verbose = false;
};

auto print_settings(const MatchSettings &settings) noexcept -> void;

[[nodiscard]] auto get_settings(const std::string &path) -> MatchSettings;

#endif
