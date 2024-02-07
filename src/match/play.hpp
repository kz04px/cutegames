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
};

auto play_game(std::shared_ptr<Game> pos,
               std::shared_ptr<Engine> engine1,
               std::shared_ptr<Engine> engine2,
               const MatchSettings &settings) -> GG;

#endif
