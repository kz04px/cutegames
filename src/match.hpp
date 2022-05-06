#ifndef MATCH_HPP
#define MATCH_HPP

struct MatchStatistics {
    // Engines
    int num_engine_loads = 0;
    int num_engine_unloads = 0;
    // Games
    int num_games_finished = 0;
    int num_games_total = 0;
    int num_p1_wins = 0;
    int num_p2_wins = 0;
    int num_draws = 0;
};

#endif
