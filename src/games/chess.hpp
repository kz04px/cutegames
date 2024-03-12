#ifndef CUTEGAMES_GAMES_CHESS_HPP
#define CUTEGAMES_GAMES_CHESS_HPP

#include <libchess/position.hpp>
#include "game.hpp"

class [[nodiscard]] ChessGame final : public Game {
   public:
    [[nodiscard]] explicit ChessGame(const std::string &fen) : Game(fen) {
        m_pos.set_fen(fen);
        m_turn = m_pos.turn() == libchess::Side::White ? Side::Player1 : Side::Player2;
        m_first_mover = m_turn;
    }

    ~ChessGame() override = default;

    void makemove(const std::string &movestr) override {
        m_move_history.emplace_back(movestr);
        const auto move = m_pos.parse_move(movestr);
        m_pos.makemove(move);
    }

    [[nodiscard]] auto is_p1_turn(std::shared_ptr<Engine>) const -> bool override {
        return m_pos.turn() == libchess::Side::White;
    }

    [[nodiscard]] bool is_gameover(std::shared_ptr<Engine>) const noexcept override {
        return m_pos.is_terminal();
    }

    [[nodiscard]] auto is_legal_move(const std::string &, std::shared_ptr<Engine>) const noexcept -> bool override {
        return true;
    }

    [[nodiscard]] auto get_result(std::shared_ptr<Engine>) const noexcept -> std::string override {
        if (m_pos.is_draw()) {
            return "draw";
        } else if (m_pos.is_checkmate()) {
            if (m_pos.turn() == libchess::Side::White) {
                return "p2win";
            } else {
                return "p1win";
            }
        } else {
            return "none";
        }
    }

   private:
    libchess::Position m_pos;
};

#endif
