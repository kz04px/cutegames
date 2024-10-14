#include <doctest/doctest.h>
#include <engine/engine.hpp>
#include <games/ataxx.hpp>
#include <libataxx/position.hpp>
#include <match/play.hpp>
#include <match/settings.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <array>
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
        num_go_received++;
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

    int num_go_received = 0;

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

TEST_CASE("Generic - Play games") {
    const std::array<std::string, 17> fens = {
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
    const auto timecontrol = SearchSettings{};
    const auto adjudication = AdjudicationSettings{};
    const auto protocol = ProtocolSettings{};
    auto engine1 = std::make_shared<TestEngine>();
    auto engine2 = std::make_shared<TestEngine>();
    auto num_p1_wins = 0;
    auto num_p2_wins = 0;
    auto num_engine1_wins = 0;
    auto num_engine2_wins = 0;
    auto num_draws = 0;

    for (const auto &fen : fens) {
        for (const auto is_engine1_p1 : {true, false}) {
            engine1->num_go_received = 0;
            engine2->num_go_received = 0;

            const auto &p1 = is_engine1_p1 ? engine1 : engine2;
            const auto &p2 = is_engine1_p1 ? engine2 : engine1;
            const auto gg = play_game(game_type, timecontrol, adjudication, protocol, fen, p1, p2);
            const auto pos = get_final(gg.game);

            REQUIRE(gg.reason == AdjudicationReason::None);
            REQUIRE(pos.is_gameover());
            REQUIRE(gg.game->start_fen() == fen);
            REQUIRE(std::abs(engine1->num_go_received - engine2->num_go_received) <= 1);

            // Game result
            switch (gg.result) {
                case GameResult::Player1Win:
                    REQUIRE_EQ(pos.get_result(), libataxx::Result::BlackWin);
                    num_p1_wins++;
                    num_engine1_wins += is_engine1_p1;
                    num_engine2_wins += !is_engine1_p1;
                    break;
                case GameResult::Player2Win:
                    REQUIRE_EQ(pos.get_result(), libataxx::Result::WhiteWin);
                    num_p2_wins++;
                    num_engine1_wins += !is_engine1_p1;
                    num_engine2_wins += is_engine1_p1;
                    break;
                case GameResult::Draw:
                    REQUIRE_EQ(pos.get_result(), libataxx::Result::Draw);
                    num_draws++;
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

        REQUIRE(num_engine1_wins == num_engine2_wins);
    }

    REQUIRE(num_p1_wins + num_p2_wins + num_draws == 2 * fens.size());
    REQUIRE(num_draws == 0);
}

TEST_CASE("Generic - Player 1 wins") {
    const std::array<std::string, 4> fens = {
        "xxxxx1o/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx x 0 1",
        "xxxxx1o/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx o 0 1",
        "xxxxx1o/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx x 99 1",
        "xxxxx1o/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx o 99 1",
    };

    const auto game_type = GameType::Ataxx;
    const auto timecontrol = SearchSettings{};
    const auto adjudication = AdjudicationSettings{};
    const auto protocol = ProtocolSettings{};
    auto engine1 = std::make_shared<TestEngine>();
    auto engine2 = std::make_shared<TestEngine>();
    auto num_engine1_wins = 0;
    auto num_engine2_wins = 0;

    for (const auto &fen : fens) {
        for (const auto is_engine1_p1 : {true, false}) {
            engine1->num_go_received = 0;
            engine2->num_go_received = 0;

            const auto &p1 = is_engine1_p1 ? engine1 : engine2;
            const auto &p2 = is_engine1_p1 ? engine2 : engine1;
            const auto gg = play_game(game_type, timecontrol, adjudication, protocol, fen, p1, p2);
            const auto pos = get_final(gg.game);

            REQUIRE(gg.reason == AdjudicationReason::None);
            REQUIRE(pos.is_gameover());
            REQUIRE(gg.result == GameResult::Player1Win);
            REQUIRE(pos.get_result() == libataxx::Result::BlackWin);
            REQUIRE(gg.game->start_fen() == fen);
            REQUIRE(std::abs(engine1->num_go_received - engine2->num_go_received) <= 1);

            num_engine1_wins += is_engine1_p1;
            num_engine2_wins += !is_engine1_p1;
        }

        REQUIRE(num_engine1_wins == num_engine2_wins);
    }

    REQUIRE(num_engine1_wins == fens.size());
}

TEST_CASE("Generic - Player 2 wins") {
    const std::array<std::string, 4> fens = {
        "ooooo1x/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo x 0 1",
        "ooooo1x/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo o 0 1",
        "ooooo1x/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo x 99 1",
        "ooooo1x/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo/ooooooo o 99 1",
    };

    const auto game_type = GameType::Ataxx;
    const auto timecontrol = SearchSettings{};
    const auto adjudication = AdjudicationSettings{};
    const auto protocol = ProtocolSettings{};
    auto engine1 = std::make_shared<TestEngine>();
    auto engine2 = std::make_shared<TestEngine>();
    auto num_engine1_wins = 0;
    auto num_engine2_wins = 0;

    for (const auto &fen : fens) {
        for (const auto is_engine1_p1 : {true, false}) {
            engine1->num_go_received = 0;
            engine2->num_go_received = 0;

            const auto &p1 = is_engine1_p1 ? engine1 : engine2;
            const auto &p2 = is_engine1_p1 ? engine2 : engine1;
            const auto gg = play_game(game_type, timecontrol, adjudication, protocol, fen, p1, p2);
            const auto pos = get_final(gg.game);

            REQUIRE(gg.reason == AdjudicationReason::None);
            REQUIRE(pos.is_gameover());
            REQUIRE(gg.result == GameResult::Player2Win);
            REQUIRE(pos.get_result() == libataxx::Result::WhiteWin);
            REQUIRE(gg.game->start_fen() == fen);
            REQUIRE(std::abs(engine1->num_go_received - engine2->num_go_received) <= 1);

            num_engine1_wins += !is_engine1_p1;
            num_engine2_wins += is_engine1_p1;
        }

        REQUIRE(num_engine1_wins == num_engine2_wins);
    }

    REQUIRE(num_engine1_wins == fens.size());
}

TEST_CASE("Generic - Draw") {
    const std::array<std::string, 16> fens = {
        "o1-----/-------/-------/-------/-------/-------/x1----- x 0 1",
        "o1-----/-------/-------/-------/-------/-------/x1----- o 0 1",
        "x------/-------/-------/-------/-------/-------/o------ x 0 1",
        "x------/-------/-------/-------/-------/-------/o------ o 0 1",
        "x-1----/-------/-------/-------/-------/-------/o------ x 0 1",
        "x-1----/-------/-------/-------/-------/-------/o------ o 0 1",
        "x------/-------/-------/-------/-------/-------/o-1---- x 0 1",
        "x------/-------/-------/-------/-------/-------/o-1---- o 0 1",
        "x-1----/-------/-------/-------/-------/-------/o-1---- x 0 1",
        "x-1----/-------/-------/-------/-------/-------/o-1---- o 0 1",
        "xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/-------/-------/o-1---- x 0 1",
        "xxxxxxx/xxxxxxx/xxxxxxx/xxxxxxx/-------/-------/o-1---- o 0 1",
        "ooooooo/ooooooo/ooooooo/ooooooo/-------/-------/x-1---- x 0 1",
        "ooooooo/ooooooo/ooooooo/ooooooo/-------/-------/x-1---- o 0 1",
        "x5o/7/7/7/7/7/o5x x 100 1",
        "x5o/7/7/7/7/7/o5x o 100 1",
    };

    const auto game_type = GameType::Generic;
    const auto timecontrol = SearchSettings{};
    const auto adjudication = AdjudicationSettings{};
    const auto protocol = ProtocolSettings{};
    auto engine1 = std::make_shared<TestEngine>();
    auto engine2 = std::make_shared<TestEngine>();

    for (const auto &fen : fens) {
        for (const auto is_engine1_p1 : {true, false}) {
            engine1->num_go_received = 0;
            engine2->num_go_received = 0;

            const auto &p1 = is_engine1_p1 ? engine1 : engine2;
            const auto &p2 = is_engine1_p1 ? engine2 : engine1;
            const auto gg = play_game(game_type, timecontrol, adjudication, protocol, fen, p1, p2);
            const auto pos = get_final(gg.game);

            REQUIRE(gg.reason == AdjudicationReason::None);
            REQUIRE(pos.is_gameover());
            REQUIRE(gg.result == GameResult::Draw);
            REQUIRE(pos.get_result() == libataxx::Result::Draw);
            REQUIRE(gg.game->start_fen() == fen);
            REQUIRE(std::abs(engine1->num_go_received - engine2->num_go_received) <= 1);
        }
    }
}
