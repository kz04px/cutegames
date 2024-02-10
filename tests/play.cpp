#include <doctest/doctest.h>
#include <engine/engine_process.hpp>
#include <games/ataxx.hpp>
#include <libataxx/position.hpp>
#include <match/play.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include "games/game.hpp"

class TestEngine final : public Engine {
   public:
    virtual ~TestEngine() {
    }

    [[nodiscard]] virtual auto is_running() -> bool override {
        return true;
    }

    virtual auto init() -> void override {
    }

    virtual auto is_ready() -> void override {
    }

    virtual auto newgame() -> void override {
        m_pos.set_fen("startpos");
    }

    virtual auto quit() -> void override {
    }

    virtual auto stop() -> void override {
    }

    virtual auto position(const std::string &start_fen, const std::vector<std::string> &move_history) -> void override {
        m_pos.set_fen(start_fen);
        for (const auto &movestr : move_history) {
            const auto move = libataxx::Move::from_uai(movestr);
            m_pos.makemove(move);
        }
    }

    virtual auto set_option(const std::string &, const std::string &) -> void override {
    }

    [[nodiscard]] virtual auto go(const SearchSettings &) -> std::string override {
        const auto moves = m_pos.legal_moves();
        std::stringstream ss;
        ss << moves.at(0);
        return ss.str();
    }

    [[nodiscard]] virtual auto query_p1turn() -> bool override {
        return m_pos.get_turn() == libataxx::Side::Black;
    }

    [[nodiscard]] virtual auto query_gameover() -> bool override {
        return m_pos.is_gameover();
    }

    [[nodiscard]] virtual auto query_result() -> std::string override {
        switch (m_pos.get_result()) {
            case libataxx::Result::BlackWin:
                return "p1win";
            case libataxx::Result::WhiteWin:
                return "p2win";
            case libataxx::Result::Draw:
                return "draw";
            case libataxx::Result::None:
                return "none";
            default:
                throw std::runtime_error("Invalid game result");
        }
    }

   private:
    libataxx::Position m_pos;
};

[[nodiscard]] static auto get_final(const std::shared_ptr<Game> game) -> libataxx::Position {
    auto pos = libataxx::Position(game->start_fen());
    for (const auto &movestr : game->move_history()) {
        const auto move = libataxx::Move::from_uai(movestr);

        REQUIRE(!pos.is_gameover());
        REQUIRE_EQ(pos.get_result(), libataxx::Result::None);
        REQUIRE(pos.is_legal_move(move));

        pos.makemove(move);
    }
    return pos;
}

TEST_CASE("Ataxx - Play games") {
    const std::array fens = {
        "startpos",
        "x5o/7/7/7/7/7/o5x x 0 1",
        "x5o/7/7/7/7/7/o5x o 0 1",
        "x5o/7/2-1-2/7/2-1-2/7/o5x x 0 1",
        "x5o/7/2-1-2/7/2-1-2/7/o5x o 0 1",
        "x5o/7/2-1-2/3-3/2-1-2/7/o5x x 0 1",
        "x5o/7/2-1-2/3-3/2-1-2/7/o5x o 0 1",
        "x5o/7/3-3/2-1-2/3-3/7/o5x x 0 1",
        "x5o/7/3-3/2-1-2/3-3/7/o5x o 0 1",
        "7/7/7/7/ooooooo/ooooooo/xxxxxxx x 0 1",
        "7/7/7/7/ooooooo/ooooooo/xxxxxxx o 0 1",
        "7/7/7/7/xxxxxxx/xxxxxxx/ooooooo x 0 1",
        "7/7/7/7/xxxxxxx/xxxxxxx/ooooooo o 0 1",
        "7/7/7/2x1o2/7/7/7 x 0 1",
        "7/7/7/2x1o2/7/7/7 o 0 1",
        "7/7/7/7/-------/-------/x5o x 0 1",
        "7/7/7/7/-------/-------/x5o o 0 1",
    };

    const auto game_type = GameType::Ataxx;
    const auto settings = MatchSettings{};
    auto engine1 = std::make_shared<TestEngine>();
    auto engine2 = std::make_shared<TestEngine>();

    for (const auto &fen : fens) {
        for (const auto &[p1, p2] : {std::make_pair(engine1, engine2), std::make_pair(engine2, engine1)}) {
            const auto gg = play_game(game_type, settings, fen, p1, p2);
            const auto pos = get_final(gg.game);

            REQUIRE(gg.reason == AdjudicationReason::None);
            REQUIRE(pos.is_gameover());
            REQUIRE(gg.game->start_fen() == fen);

            // Game result
            switch (gg.result) {
                case GameResult::Player1Win:
                    REQUIRE_EQ(pos.get_result(), libataxx::Result::BlackWin);
                    break;
                case GameResult::Player2Win:
                    REQUIRE_EQ(pos.get_result(), libataxx::Result::WhiteWin);
                    break;
                case GameResult::Draw:
                    REQUIRE_EQ(pos.get_result(), libataxx::Result::Draw);
                    break;
                case GameResult::None:
                    REQUIRE_EQ(pos.get_result(), libataxx::Result::None);
                    FAIL("No game result");
                    break;
                default:
                    FAIL("Unknown game result");
                    break;
            }
        }
    }
}

TEST_CASE("Ataxx - Black wins") {
    const std::array fens = {
        "xxxxx1o/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx x 0 1",
        "xxxxx1o/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx o 0 1",
        "xxxxx1o/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx x 99 1",
        "xxxxx1o/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx o 99 1",
    };

    const auto game_type = GameType::Ataxx;
    const auto settings = MatchSettings{};
    auto engine1 = std::make_shared<TestEngine>();
    auto engine2 = std::make_shared<TestEngine>();

    for (const auto &fen : fens) {
        for (const auto &[p1, p2] : {std::make_pair(engine1, engine2), std::make_pair(engine2, engine1)}) {
            const auto gg = play_game(game_type, settings, fen, p1, p2);
            const auto pos = get_final(gg.game);

            REQUIRE(gg.reason == AdjudicationReason::None);
            REQUIRE(pos.is_gameover());
            REQUIRE(gg.result == GameResult::Player1Win);
            REQUIRE(pos.get_result() == libataxx::Result::BlackWin);
            REQUIRE(gg.game->start_fen() == fen);
        }
    }
}

TEST_CASE("Ataxx - White wins") {
    const std::array fens = {
        "ooooo1x/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo x 0 1",
        "ooooo1x/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo o 0 1",
        "ooooo1x/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo x 99 1",
        "ooooo1x/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo o 99 1",
    };

    const auto game_type = GameType::Ataxx;
    const auto settings = MatchSettings{};
    auto engine1 = std::make_shared<TestEngine>();
    auto engine2 = std::make_shared<TestEngine>();

    for (const auto &fen : fens) {
        for (const auto &[p1, p2] : {std::make_pair(engine1, engine2), std::make_pair(engine2, engine1)}) {
            const auto gg = play_game(game_type, settings, fen, p1, p2);
            const auto pos = get_final(gg.game);

            REQUIRE(gg.reason == AdjudicationReason::None);
            REQUIRE(pos.is_gameover());
            REQUIRE(gg.result == GameResult::Player2Win);
            REQUIRE(pos.get_result() == libataxx::Result::WhiteWin);
            REQUIRE(gg.game->start_fen() == fen);
        }
    }
}
