#include "play.hpp"
#include "engine/engine_uai.hpp"
#include "engine/engine_ugi.hpp"
#include "events/events.hpp"
#include "games/game.hpp"

[[nodiscard]] auto get_engine(const MatchSettings &settings,
                              libevents::Dispatcher &dispatcher,
                              Store<Engine> &engine_store,
                              const std::size_t id) -> std::shared_ptr<Engine> {
    auto e = engine_store.get([id](const auto &obj) noexcept -> bool {
        return id == obj->get_id();
    });

    // Return engine from cache
    if (e) {
        return *e;
    }

    dispatcher.post_event(
        std::make_shared<EngineCreated>(id, settings.engine_settings[id].name, settings.engine_settings[id].path));

    // Return new engine instance
    auto ptr = [&settings, id]() -> std::shared_ptr<Engine> {
        switch (settings.engine_settings[id].protocol) {
            case EngineProtocol::UGI:
                return make_engine<UGIEngine>(settings.engine_settings[id], settings.debug);
            case EngineProtocol::UAI:
                return make_engine<UAIEngine>(settings.engine_settings[id], settings.debug);
            default:
                return {};
        }
    }();
    ptr->init();

    // Set options
    for (const auto &[name, value] : settings.engine_settings[id].options) {
        ptr->set_option(name, value);
    }

    return ptr;
}

auto play_game(std::shared_ptr<Game> pos,
               const std::size_t game_id,
               const std::string fen,
               const std::size_t engine1_id,
               const std::size_t engine2_id,
               const MatchSettings &settings,
               libevents::Dispatcher &dispatcher,
               Store<Engine> &engine_store) -> void {
    // Game started
    dispatcher.post_event(std::make_shared<GameStarted>(game_id, fen, engine1_id, engine2_id));

    auto engine1 = get_engine(settings, dispatcher, engine_store, engine1_id);
    auto engine2 = get_engine(settings, dispatcher, engine_store, engine2_id);

    engine1->is_ready();
    engine2->is_ready();

    engine1->newgame();
    engine2->newgame();

    auto tc = settings.timecontrol;
    auto out_of_time = false;
    auto gameover_claimed = false;

    // Find out whose turn it is
    {
        const auto is_p1_turn = pos->is_p1_turn(engine1);
        pos->set_turn(is_p1_turn ? Side::Player1 : Side::Player2);
    }

    // Set who moved first for later use in the .pgn
    pos->set_first_mover(pos->turn());

    // Play a game
    while (true) {
        // Check if we should ask the engine whose turn it is
        if (settings.protocol.ask_turn) {
            const auto is_p1_turn = pos->is_p1_turn(engine1);
            pos->set_turn(is_p1_turn ? Side::Player1 : Side::Player2);
        }

        const auto is_p1_turn = pos->turn() == Side::Player1;
        auto &us = is_p1_turn ? engine1 : engine2;

        // Inform the engine of the current position
        us->is_ready();
        us->position(pos->start_fen(), pos->move_history());

        // Ask if the game is over
        if (pos->is_gameover(us)) {
            gameover_claimed = true;
            break;
        }

        // Should we ask the other engine if the game is over?
        if (settings.protocol.gameover == QueryGameover::Both) {
            auto &them = is_p1_turn ? engine2 : engine1;
            them->is_ready();
            if (pos->is_gameover(them)) {
                gameover_claimed = true;
                break;
            }
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

        pos->makemove(movestr);

        // Assume that the side to move alternates with each move
        // If this is not the case, then the "askturn" protocol setting should be set to true
        pos->set_turn(!pos->turn());
    }

    auto result = GameResult::None;
    auto adjudicated = AdjudicationReason::None;

    if (out_of_time) {
        result = pos->turn() == Side::Player1 ? GameResult::Player2Win : GameResult::Player1Win;
        adjudicated = AdjudicationReason::Timeout;
    } else if (gameover_claimed) {
        engine1->is_ready();
        engine1->position(pos->start_fen(), pos->move_history());
        const auto gameover1 = pos->is_gameover(engine1);
        const auto result1 = pos->get_result(engine1);

        engine2->is_ready();
        engine2->position(pos->start_fen(), pos->move_history());
        const auto gameover2 = pos->is_gameover(engine2);
        const auto result2 = pos->get_result(engine2);

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

    // Return the engines now we're done with them
    const auto released1 = engine_store.release(engine1);
    const auto released2 = engine_store.release(engine2);

    engine1.reset();
    engine2.reset();

    if (released1) {
        dispatcher.post_event(std::make_shared<EngineDestroyed>(99, "", ""));
    }
    if (released2) {
        dispatcher.post_event(std::make_shared<EngineDestroyed>(99, "", ""));
    }

    dispatcher.post_event(std::make_shared<GameFinished>(game_id, engine1_id, engine2_id, result, adjudicated, pos));
}