#include "play.hpp"
#include "games/ataxx.hpp"
#include "games/game.hpp"
#include "games/ugigame.hpp"

[[nodiscard]] auto make_game(const GameType game_type, const std::string fen = "startpos") -> std::shared_ptr<Game> {
    switch (game_type) {
        case GameType::Generic:
            return std::make_shared<UGIGame>(fen);
        case GameType::Ataxx:
            return std::make_shared<AtaxxGame>(fen);
        default:
            throw std::invalid_argument("Unrecognised game type");
    }
}

auto play_game(const GameType game_type,
               const MatchSettings &settings,
               const std::string &fen,
               std::shared_ptr<Engine> engine1,
               std::shared_ptr<Engine> engine2) -> GG {
    auto game = make_game(game_type, fen);

    engine1->is_ready();
    engine2->is_ready();

    engine1->newgame();
    engine2->newgame();

    auto tc = settings.timecontrol;
    auto out_of_time = false;
    auto gameover_claimed = false;

    // Find out whose turn it is
    if (game_type == GameType::Generic) {
        const auto is_p1_turn = game->is_p1_turn(engine1);
        const auto to_move = is_p1_turn ? Side::Player1 : Side::Player2;
        game->set_turn(to_move);
        game->set_first_mover(to_move);
    }

    // Play a game
    while (true) {
        // Check if we should ask the engine whose turn it is
        if (game_type == GameType::Generic && settings.protocol.ask_turn) {
            const auto is_p1_turn = game->is_p1_turn(engine1);
            game->set_turn(is_p1_turn ? Side::Player1 : Side::Player2);
        }

        const auto is_p1_turn = game->turn() == Side::Player1;
        auto &us = is_p1_turn ? engine1 : engine2;
        auto &them = is_p1_turn ? engine2 : engine1;

        // Inform the engine of the current position
        us->is_ready();
        us->position(game->start_fen(), game->move_history());

        // Ask if the game is over
        if (game->is_gameover(us) || (game_type == GameType::Generic &&
                                      settings.protocol.gameover == QueryGameover::Both && game->is_gameover(them))) {
            gameover_claimed = true;
            break;
        }

        const auto t0 = std::chrono::steady_clock::now();

        // Get move string
        const auto movestr = us->go(tc);

        const auto t1 = std::chrono::steady_clock::now();
        const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);

        // Check time usage
        switch (tc.type) {
            case SearchSettings::Type::Time:
                if (is_p1_turn) {
                    tc.p1time -= dt.count();
                    if (tc.p1time <= 0) {
                        out_of_time = true;
                        break;
                    }
                    tc.p1time += tc.p1inc;
                } else {
                    tc.p2time -= dt.count();
                    if (tc.p2time <= 0) {
                        out_of_time = true;
                        break;
                    }
                    tc.p2time += tc.p2inc;
                }

                break;
            case SearchSettings::Type::Movetime:
                if (dt.count() > tc.movetime + settings.adjudication.timeoutbuffer) {
                    out_of_time = true;
                }
                break;
            case SearchSettings::Type::Depth:
                break;
            case SearchSettings::Type::Nodes:
                break;
            default:
                break;
        }

        if (out_of_time) {
            break;
        }

        game->makemove(movestr);
    }

    auto result = GameResult::None;
    auto adjudicated = AdjudicationReason::None;

    if (out_of_time) {
        result = game->turn() == Side::Player1 ? GameResult::Player2Win : GameResult::Player1Win;
        adjudicated = AdjudicationReason::Timeout;
    } else if (gameover_claimed) {
        engine1->is_ready();
        engine1->position(game->start_fen(), game->move_history());
        const auto gameover1 = game->is_gameover(engine1);
        const auto result1 = game->get_result(engine1);

        engine2->is_ready();
        engine2->position(game->start_fen(), game->move_history());
        const auto gameover2 = game->is_gameover(engine2);
        const auto result2 = game->get_result(engine2);

        if (gameover1 != gameover2) {
            adjudicated = AdjudicationReason::GameoverMismatch;
        } else if (result1 != result2) {
            adjudicated = AdjudicationReason::ResultMismatch;
        } else {
            if (result1 == "p1win") {
                result = GameResult::Player1Win;
            } else if (result1 == "p2win") {
                result = GameResult::Player2Win;
            } else if (result1 == "draw") {
                result = GameResult::Draw;
            } else {
                result = GameResult::None;
            }
        }
    }

    return GG{result, adjudicated, game};
}
