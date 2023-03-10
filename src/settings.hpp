#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstdlib>
#include <string>
#include <vector>
#include "engine/engine.hpp"

struct SPRTSettings {
    bool enabled = false;
    float alpha = 0.05f;
    float beta = 0.05f;
    float elo0 = 0.0f;
    float elo1 = 5.0f;
};

struct AdjudicationSettings {
    int timeoutbuffer = 10;
    int maxfullmoves = 0;
};

struct MatchSettings {
    std::size_t num_threads = 1;
    int num_games = 1;
    int engine_store_size = 2;
    int update_frequency = 10;
    std::string openings_path;
    std::vector<EngineSettings> engine_settings;
    SearchSettings timecontrol;
    SPRTSettings sprt;
    AdjudicationSettings adjudication;
    bool shuffle_openings = false;
    bool repeat = true;
    bool debug = false;
    bool recover = true;
    bool verbose = false;
};

auto print_settings(const MatchSettings &settings) noexcept -> void;

[[nodiscard]] auto get_settings(const std::string &path) -> MatchSettings;

#endif
