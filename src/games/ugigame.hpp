#ifndef UGI_GAME_HPP
#define UGI_GAME_HPP

#include "game.hpp"

class [[nodiscard]] UGIGame final : public Game {
   public:
    [[nodiscard]] explicit UGIGame(const std::string &fen) : Game(fen) {
    }

    ~UGIGame() override = default;

    void makemove(const std::string &movestr) override {
        m_move_history.emplace_back(movestr);
        m_turn = !m_turn;
    }

    [[nodiscard]] auto is_p1_turn(std::shared_ptr<Engine> engine) const -> bool override {
        engine->position(start_fen(), move_history());
        return engine->query_p1turn();
    }

    [[nodiscard]] bool is_gameover(std::shared_ptr<Engine> engine) const noexcept override {
        engine->position(start_fen(), move_history());
        return engine->query_gameover();
    }

    [[nodiscard]] auto is_legal_move(const std::string &, std::shared_ptr<Engine>) const noexcept -> bool override {
        return true;
    }

    [[nodiscard]] auto get_result(std::shared_ptr<Engine> engine) const noexcept -> std::string override {
        engine->position(start_fen(), move_history());
        return engine->query_result();
    }
};

#endif
