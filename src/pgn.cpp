#include "pgn.hpp"
#include <fstream>
#include "games/game.hpp"
#include "games/ugigame.hpp"

[[nodiscard]] auto result_string(const GameResult result) -> std::string {
    switch (result) {
        case GameResult::Player1Win:
            return "1-0";
        case GameResult::Player2Win:
            return "0-1";
        case GameResult::Draw:
            return "1/2-1/2";
        default:
            return "*";
    }
}

[[nodiscard]] auto adjudication_string(const AdjudicationReason reason) -> std::string {
    switch (reason) {
        case AdjudicationReason::Timeout:
            return "Out of time";
        case AdjudicationReason::Resign:
            return "Resigned";
        case AdjudicationReason::Crash:
            return "Crashed";
        case AdjudicationReason::IllegalMove:
            return "Illegal move";
        case AdjudicationReason::Gamelength:
            return "Maximum game length";
        case AdjudicationReason::GameoverMismatch:
            return "Gameover mismatch";
        case AdjudicationReason::ResultMismatch:
            return "Result mismatch";
        case AdjudicationReason::None:
            return "*";
        default:
            return "*";
    }
}

auto write_as_pgn(const PGNSettings &settings,
                  const std::string &player1,
                  const std::string &player2,
                  const GameResult result,
                  const AdjudicationReason reason,
                  const UGIGame &game) -> void {
    std::ofstream f(settings.path, std::fstream::out | std::fstream::app);
    if (!f.is_open()) {
        return;
    }

    f << "[Event \"" << settings.event << "\"]\n";
    f << "[Site \"CuteGames\"]\n";
    f << "[Date \"??\"]\n";
    f << "[Round \"1\"]\n";
    f << "[" << settings.colour1 << " \"" << player1 << "\"]\n";
    f << "[" << settings.colour2 << " \"" << player2 << "\"]\n";
    f << "[Result \"" << result_string(result) << "\"]\n";
    f << "[FEN \"" << game.start_fen() << "\"]\n";
    if (reason != AdjudicationReason::None) {
        f << "[Adjudicated \"" << adjudication_string(reason) << "\"]\n";
    }
    if (result == GameResult::Player1Win) {
        f << "[Winner \"" << player1 << "\"]\n";
        f << "[Loser \"" << player2 << "\"]\n";
    } else if (result == GameResult::Player2Win) {
        f << "[Winner \"" << player2 << "\"]\n";
        f << "[Loser \"" << player1 << "\"]\n";
    }
    f << "[PlyCount \"" << game.move_history().size() << "\"]\n";
    f << "\n";

    auto ply = 0;

    const auto p2_first = game.get_first_mover() == Side::Player2;
    if (p2_first) {
        f << "1... ";
        ply++;
    }

    for (const auto &movestr : game.move_history()) {
        if (ply % 2 == 0) {
            f << ply / 2 + 1 << ". ";
        }

        f << movestr << " ";

        ply++;
    }

    f << result_string(result) << "\n";

    f << "\n\n";
}
