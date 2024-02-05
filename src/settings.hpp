#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstdlib>
#include <string>
#include <vector>
#include "engine/engine.hpp"
#include "games/game.hpp"
#include "tournament/types.hpp"

enum class QueryGameover
{
    Tomove = 0,
    Both,
};

struct [[nodiscard]] SPRTSettings {
    bool enabled = false;
    float alpha = 0.05f;
    float beta = 0.05f;
    float elo0 = 0.0f;
    float elo1 = 5.0f;
};

struct [[nodiscard]] AdjudicationSettings {
    int timeoutbuffer = 10;
    int maxfullmoves = 0;
};

struct [[nodiscard]] ProtocolSettings {
    QueryGameover gameover = QueryGameover::Tomove;
    bool ask_turn = false;
};

struct [[nodiscard]] PGNSettings {
    std::string path = "games.pgn";
    std::string event = "*";
    std::string colour1 = "white";
    std::string colour2 = "black";
    bool enabled = true;
    bool verbose = false;
    bool override = false;
};

struct [[nodiscard]] MatchSettings {
    GameType game_type = GameType::Generic;
    std::size_t num_threads = 1;
    int num_games = 1;
    int engine_store_size = 2;
    int update_frequency = 10;
    std::string openings_path;
    TournamentType tournament_type = TournamentType::RoundRobin;
    std::vector<EngineSettings> engine_settings;
    SearchSettings timecontrol;
    SPRTSettings sprt;
    PGNSettings pgn;
    AdjudicationSettings adjudication;
    ProtocolSettings protocol;
    bool shuffle_openings = false;
    bool repeat = true;
    bool debug = false;
    bool recover = true;
    bool verbose = false;
};

auto print_settings(const MatchSettings &settings) noexcept -> void;

[[nodiscard]] auto get_settings(const std::string &path) -> MatchSettings;

#endif
