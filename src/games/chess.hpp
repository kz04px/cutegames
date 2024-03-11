#ifndef CUTEGAMES_GAMES_CHESS_HPP
#define CUTEGAMES_GAMES_CHESS_HPP

#include <exception>
#include <iostream>
#include <libchess/position.hpp>
#include "game.hpp"

class ChessGame final : public Game {
   public:
    [[nodiscard]] ChessGame() : Game() {
    }

    [[nodiscard]] ChessGame(const std::string &fen) : Game(fen) {
        pos.set_fen(fen);
        m_turn = pos.turn() == libchess::Side::White ? Side::Player1 : Side::Player2;
        m_first_mover = m_turn;
    }

    virtual ~ChessGame() = default;

    virtual void makemove(const std::string &movestr) override {
        m_move_history.emplace_back(movestr);
        try {
            const auto move = pos.parse_move(movestr);
            pos.makemove(move);
        } catch (std::exception &e) {
            std::cerr << "Move: ~" << movestr << "~" << std::endl;
            std::cerr << "Exception: " << e.what() << std::endl;
            std::exit(1);
        }
    }

    [[nodiscard]] virtual auto is_p1_turn(std::shared_ptr<Engine>) const -> bool override {
        return pos.turn() == libchess::Side::White;
    }

    [[nodiscard]] virtual bool is_gameover(std::shared_ptr<Engine>) const noexcept override {
        return pos.is_terminal();
    }

    [[nodiscard]] virtual auto is_legal_move(const std::string &,
                                             std::shared_ptr<Engine>) const noexcept -> bool override {
        return true;
    }

    [[nodiscard]] virtual auto get_result(std::shared_ptr<Engine>) const noexcept -> std::string override {
        if (pos.is_draw()) {
            return "draw";
        } else if (pos.is_checkmate()) {
            if (pos.turn() == libchess::Side::White) {
                return "p2win";
            } else {
                return "p1win";
            }
        } else {
            return "none";
        }
    }

   protected:
    libchess::Position pos;
};

#endif
