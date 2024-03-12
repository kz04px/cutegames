#ifndef CUTEGAMES_GAMES_ATAXX_HPP
#define CUTEGAMES_GAMES_ATAXX_HPP

#include <libataxx/position.hpp>
#include "game.hpp"

class [[nodiscard]] AtaxxGame final : public Game {
   public:
    [[nodiscard]] explicit AtaxxGame(const std::string &fen) : Game(fen) {
        m_pos.set_fen(fen);
        m_turn = m_pos.get_turn() == libataxx::Side::Black ? Side::Player1 : Side::Player2;
        m_first_mover = m_turn;
    }

    ~AtaxxGame() override = default;

    void makemove(const std::string &movestr) override {
        m_move_history.emplace_back(movestr);
        const auto move = libataxx::Move::from_uai(movestr);
        m_pos.makemove(move);
    }

    [[nodiscard]] auto is_p1_turn(std::shared_ptr<Engine>) const -> bool override {
        return m_pos.get_turn() == libataxx::Side::Black;
    }

    [[nodiscard]] bool is_gameover(std::shared_ptr<Engine>) const noexcept override {
        return m_pos.is_gameover();
    }

    [[nodiscard]] auto is_legal_move(const std::string &, std::shared_ptr<Engine>) const noexcept -> bool override {
        return true;
    }

    [[nodiscard]] auto get_result(std::shared_ptr<Engine>) const noexcept -> std::string override {
        switch (m_pos.get_result()) {
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

   private:
    libataxx::Position m_pos;
};

#endif
