#ifndef TOURNAMENT_GAUNTLET_HPP
#define TOURNAMENT_GAUNTLET_HPP

#include <cstddef>
#include "generator.hpp"

class [[nodiscard]] GauntletGenerator final : public TournamentGenerator {
   public:
    GauntletGenerator(const std::size_t players, const std::size_t games, const std::size_t openings, const bool r)
        : num_players(players), num_games(games), num_openings(openings), repeat(r) {
    }

    ~GauntletGenerator() override = default;

    [[nodiscard]] auto is_finished() -> bool override {
        return idx >= expected();
    }

    [[nodiscard]] auto expected() -> std::size_t override {
        return num_games * (num_players - 1);
    }

    [[nodiscard]] auto next() -> GameInfo override {
        GameInfo result;

        const auto is_mirror = match_game % 2 == 1;
        if (is_mirror && repeat) {
            result = GameInfo{idx, opening, player2, 0};
        } else {
            result = GameInfo{idx, opening, 0, player2};
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
            player2 = 1;
        }

        if (repeat) {
            opening = (match_game / 2) % num_openings;
        } else {
            opening = match_game % num_openings;
        }
    }

    std::size_t num_players = 0;
    std::size_t num_games = 0;
    std::size_t num_openings = 0;
    bool repeat = true;
    // state
    std::size_t idx = 0;
    std::size_t opening = 0;
    std::size_t player2 = 1;
    std::size_t match_game = 0;
};

#endif
