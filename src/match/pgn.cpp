#include "pgn.hpp"
#include <fstream>
#include "games/game.hpp"

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
                  const std::shared_ptr<Game> &game) -> void {
    std::ofstream file(settings.path, std::fstream::out | std::fstream::app);
    if (!file.is_open()) {
        return;
    }

    file << "[Event \"" << settings.event << "\"]\n";
    file << "[Site \"CuteGames\"]\n";
    file << "[Date \"??\"]\n";
    file << "[Round \"1\"]\n";
    file << "[" << settings.colour1 << " \"" << player1 << "\"]\n";
    file << "[" << settings.colour2 << " \"" << player2 << "\"]\n";
    file << "[Result \"" << result_string(result) << "\"]\n";
    file << "[FEN \"" << game->start_fen() << "\"]\n";
    if (reason != AdjudicationReason::None) {
        file << "[Adjudicated \"" << adjudication_string(reason) << "\"]\n";
    }
    if (result == GameResult::Player1Win) {
        file << "[Winner \"" << player1 << "\"]\n";
        file << "[Loser \"" << player2 << "\"]\n";
    } else if (result == GameResult::Player2Win) {
        file << "[Winner \"" << player2 << "\"]\n";
        file << "[Loser \"" << player1 << "\"]\n";
    }
    file << "[PlyCount \"" << game->move_history().size() << "\"]\n";
    file << "\n";

    auto ply = 0;

    const auto p2_first = game->get_first_mover() == Side::Player2;
    if (p2_first) {
        file << "1... ";
        ply++;
    }

    for (const auto &movestr : game->move_history()) {
        if (ply % 2 == 0) {
            file << ply / 2 + 1 << ". ";
        }

        file << movestr << " ";

        ply++;
    }

    file << result_string(result) << "\n";

    file << "\n\n";
}
