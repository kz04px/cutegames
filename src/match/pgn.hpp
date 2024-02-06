#ifndef MATCH_PGN_HPP
#define MATCH_PGN_HPP

#include <memory>
#include <string>
#include "games/game.hpp"

struct [[nodiscard]] PGNSettings {
    std::string path = "games.pgn";
    std::string event = "*";
    std::string colour1 = "white";
    std::string colour2 = "black";
    bool enabled = true;
    bool verbose = false;
    bool override = false;
};

auto write_as_pgn(const PGNSettings &settings,
                  const std::string &player1,
                  const std::string &player2,
                  const GameResult result,
                  const AdjudicationReason reason,
                  const std::shared_ptr<Game> game) -> void;

#endif
