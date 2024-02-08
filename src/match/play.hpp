#ifndef MATCH_PLAY_HPP
#define MATCH_PLAY_HPP

#include <libevents.hpp>
#include <memory>
#include "engine/engine.hpp"
#include "games/game.hpp"
#include "settings.hpp"

struct [[nodiscard]] GG {
    GameResult result;
    AdjudicationReason reason;
    std::shared_ptr<Game> game;
};

auto play_game(const GameType game_type,
               const MatchSettings &settings,
               const std::string &fen,
               std::shared_ptr<Engine> engine1,
               std::shared_ptr<Engine> engine2) -> GG;

#endif
