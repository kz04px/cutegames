#ifndef TOURNAMENT_ROUNDROBIN_HPP
#define TOURNAMENT_ROUNDROBIN_HPP

#include <cassert>
#include <cstddef>
#include "generator.hpp"

class [[nodiscard]] RoundRobinGenerator final : public TournamentGenerator {
   public:
    RoundRobinGenerator(const std::size_t players, const std::size_t games, const std::size_t openings, const bool r)
        : num_players(players), num_games(games), num_openings(openings), repeat(r) {
    }

    ~RoundRobinGenerator() override = default;

    [[nodiscard]] auto is_finished() -> bool override {
        return idx >= expected();
    }

    [[nodiscard]] auto expected() -> std::size_t override {
        const auto games_per_player = num_games * (num_players - 1);
        return games_per_player * num_players / 2;
    }

    [[nodiscard]] auto next() -> GameInfo override {
        GameInfo result;

        const auto is_mirror = match_game % 2 == 1;
        if (is_mirror && repeat) {
            result = GameInfo{idx, opening, player2, player1};
        } else {
            result = GameInfo{idx, opening, player1, player2};
        }

        increment();

        return result;
    }

   private:
    auto increment() -> void override {
        idx++;
        match_game++;

        if (match_game >= num_games) {
            match_game = 0;
            player2++;
        }

        if (player2 >= num_players) {
            player1++;
            player2 = player1 + 1;
        }

        if (player1 >= num_players - 1) {
            player1 = 0;
            player2 = 1;
        }

        if (repeat) {
            opening = (match_game / 2) % num_openings;
        } else {
            opening = match_game % num_openings;
        }

        assert(player1 < num_players);
        assert(player2 < num_players);
    }

    std::size_t num_players = 0;
    std::size_t num_games = 0;
    std::size_t num_openings = 0;
    bool repeat = true;
    // state
    std::size_t idx = 0;
    std::size_t opening = 0;
    std::size_t player1 = 0;
    std::size_t player2 = 1;
    std::size_t match_game = 0;
};

#endif
