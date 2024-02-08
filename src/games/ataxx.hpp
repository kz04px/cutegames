#ifndef CUTEGAMES_GAMES_ATAXX_HPP
#define CUTEGAMES_GAMES_ATAXX_HPP

#include <libataxx/position.hpp>
#include "game.hpp"

class AtaxxGame final : public Game {
   public:
    [[nodiscard]] AtaxxGame() : Game() {
    }

    [[nodiscard]] AtaxxGame(const std::string &fen) : Game(fen) {
        pos.set_fen(fen);
        m_turn = pos.get_turn() == libataxx::Side::Black ? Side::Player1 : Side::Player2;
        m_first_mover = m_turn;
    }

    virtual ~AtaxxGame() = default;

    virtual void makemove(const std::string &movestr) override {
        m_move_history.emplace_back(movestr);
        const auto move = libataxx::Move::from_uai(movestr);
        pos.makemove(move);
    }

    [[nodiscard]] virtual auto is_p1_turn(std::shared_ptr<Engine>) const -> bool override {
        return pos.get_turn() == libataxx::Side::Black;
    }

    [[nodiscard]] virtual bool is_gameover(std::shared_ptr<Engine>) const noexcept override {
        return pos.is_gameover();
    }

    [[nodiscard]] virtual auto is_legal_move(const std::string &,
                                             std::shared_ptr<Engine>) const noexcept -> bool override {
        return true;
    }

    [[nodiscard]] virtual auto get_result(std::shared_ptr<Engine>) const noexcept -> std::string override {
        switch (pos.get_result()) {
            case libataxx::Result::BlackWin:
                return "p1win";
            case libataxx::Result::WhiteWin:
                return "p2win";
            case libataxx::Result::Draw:
                return "draw";
            default:
                return "none";
        }
    }

   protected:
    libataxx::Position pos;
};

#endif
