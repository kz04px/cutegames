#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <vector>

enum class GameResult
{
    Player1Win = 0,
    Player2Win,
    Draw,
    None,
};

enum class Side
{
    Player1 = 0,
    Player2,
};

enum class AdjudicationReason
{
    Decisive = 0,
    Timeout,
    Resign,
    Crash,
    IllegalMove,
    None,
};

[[nodiscard]] constexpr auto operator!(const Side r) noexcept -> Side {
    return static_cast<Side>(!static_cast<bool>(r));
}

static_assert(Side::Player1 == !Side::Player2);
static_assert(Side::Player2 == !Side::Player1);

class Game {
   public:
    [[nodiscard]] Game(const std::string &fen) : m_start_fen(fen) {
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

    [[nodiscard]] virtual auto is_gameover() const noexcept -> bool = 0;

    [[nodiscard]] virtual auto is_legal_move(const std::string &movestr) const noexcept -> bool = 0;

    [[nodiscard]] virtual auto get_result() const noexcept -> GameResult = 0;

    virtual auto makemove(const std::string &movestr) -> void = 0;

    auto set_turn(const Side s) noexcept -> void {
        m_turn = s;
    }

   protected:
    std::string m_start_fen;
    std::vector<std::string> m_move_history;
    Side m_turn = Side::Player1;
};

#endif
