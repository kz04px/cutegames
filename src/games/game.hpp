#ifndef GAME_HPP
#define GAME_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "../engine/engine.hpp"

enum class [[nodiscard]] GameType
{
    Generic = 0,
    Ataxx,
    Chess,
};

enum class [[nodiscard]] GameResult
{
    Player1Win = 0,
    Player2Win,
    Draw,
    None,
};

enum class [[nodiscard]] Side
{
    Player1 = 0,
    Player2,
};

enum class [[nodiscard]] AdjudicationReason
{
    Timeout = 0,
    Resign,
    Crash,
    IllegalMove,
    Gamelength,
    GameoverMismatch,
    ResultMismatch,
    None,
};

[[nodiscard]] constexpr auto operator!(const Side r) noexcept -> Side {
    return static_cast<Side>(!static_cast<bool>(r));
}

static_assert(Side::Player1 == !Side::Player2);
static_assert(Side::Player2 == !Side::Player1);

class Game {
   public:
    [[nodiscard]] Game() : m_start_fen("startpos") {
    }

    [[nodiscard]] explicit Game(std::string fen) : m_start_fen(std::move(fen)) {
    }

    virtual ~Game() = default;

    [[nodiscard]] auto move_history() const noexcept -> const std::vector<std::string> & {
        return m_move_history;
    }

    [[nodiscard]] auto start_fen() const noexcept -> const std::string & {
        return m_start_fen;
    }

    [[nodiscard]] virtual auto turn() const noexcept -> Side {
        return m_turn;
    }

    [[nodiscard]] virtual auto is_p1_turn(std::shared_ptr<Engine> engine) const -> bool = 0;

    [[nodiscard]] virtual auto is_gameover(std::shared_ptr<Engine>) const noexcept -> bool = 0;

    [[nodiscard]] virtual auto is_legal_move(const std::string &movestr, std::shared_ptr<Engine>) const noexcept
        -> bool = 0;

    [[nodiscard]] virtual auto get_result(std::shared_ptr<Engine>) const noexcept -> std::string = 0;

    [[nodiscard]] auto get_first_mover() const noexcept -> Side {
        return m_first_mover;
    }

    virtual auto makemove(const std::string &movestr) -> void = 0;

    auto set_turn(const Side side) noexcept -> void {
        m_turn = side;
    }

    auto set_first_mover(const Side side) noexcept -> void {
        m_first_mover = side;
    }

   protected:
    std::string m_start_fen;
    std::vector<std::string> m_move_history;
    Side m_turn = Side::Player1;
    Side m_first_mover = Side::Player1;
};

#endif
