#ifndef PGN_HPP
#define PGN_HPP

#include <memory>
#include <string>
#include "games/game.hpp"
#include "settings.hpp"

class UGIGame;

auto write_as_pgn(const PGNSettings &settings,
                  const std::string &player1,
                  const std::string &player2,
                  const GameResult result,
                  const AdjudicationReason reason,
                  const std::shared_ptr<Game> game) -> void;

#endif
