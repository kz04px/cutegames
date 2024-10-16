#ifndef MATCH_PLAY_HPP
#define MATCH_PLAY_HPP

#include <libevents.hpp>
#include <memory>
#include "games/game.hpp"

struct AdjudicationSettings;
struct ProtocolSettings;

struct [[nodiscard]] GG {
    GameResult result;
    AdjudicationReason reason;
    std::shared_ptr<Game> game;
};

auto play_game(const GameType game_type,
               const SearchSettings &timecontrol,
               const AdjudicationSettings &adjudication,
               const ProtocolSettings &protocol,
               const std::string &fen,
               const std::shared_ptr<Engine> &engine1,
               const std::shared_ptr<Engine> &engine2) -> GG;

#endif
