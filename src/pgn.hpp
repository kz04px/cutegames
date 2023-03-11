#ifndef PGN_HPP
#define PGN_HPP

#include <string>
#include "settings.hpp"

class UGIGame;

auto write_as_pgn(const PGNSettings &settings,
                  const std::string &player1,
                  const std::string &player2,
                  const GameResult result,
                  const AdjudicationReason reason,
                  const UGIGame &game) -> void;

#endif
