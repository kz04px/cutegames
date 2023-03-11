#include "play.hpp"
#include "games/ugigame.hpp"

[[nodiscard]] auto get_engine(const MatchSettings &settings,
                              libevents::Dispatcher &dispatcher,
                              Store<UGIEngine> &engine_store,
                              const std::size_t id) -> std::shared_ptr<UGIEngine> {
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
    auto ptr = make_engine(settings.engine_settings[id], settings.debug);
    ptr->ugi();

    // Set options
    for (const auto &[name, value] : settings.engine_settings[id].options) {
        ptr->set_option(name, value);
    }

    return ptr;
}

void play_game(const std::size_t game_id,
               const std::string fen,
               const std::size_t engine1_id,
               const std::size_t engine2_id,
               const MatchSettings &settings,
               libevents::Dispatcher &dispatcher,
               Store<UGIEngine> &engine_store) {
    // Game started
    dispatcher.post_event(std::make_shared<GameStarted>(game_id, fen, engine1_id, engine2_id));

    auto engine1 = get_engine(settings, dispatcher, engine_store, engine1_id);
    auto engine2 = get_engine(settings, dispatcher, engine_store, engine2_id);

    engine1->isready();
    engine2->isready();

    engine1->newgame();
    engine2->newgame();

    auto pos = UGIGame(fen);
    auto tc = settings.timecontrol;
    auto out_of_time = false;
    auto gameover_claimed = false;

    // Find out whose turn it is
    engine1->position(pos);
    pos.set_turn(engine1->query_p1turn() ? Side::Player1 : Side::Player2);

    // Set who moved first for later use in the .pgn
    pos.set_first_mover(pos.turn());

    // Play a game
    while (true) {
        // Check if we should ask the engine whose turn it is
        if (settings.protocol.ask_turn) {
            engine1->position(pos);
            pos.set_turn(engine1->query_p1turn() ? Side::Player1 : Side::Player2);
        }

        const auto is_p1_turn = pos.turn() == Side::Player1;
        auto &us = is_p1_turn ? engine1 : engine2;

        // Inform the engine of the current position
        us->isready();
        us->position(pos);

        // Ask if the game is over
        if (us->query_gameover()) {
            gameover_claimed = true;
            break;
        }

        // Should we ask the other engine if the game is over?
        if (settings.protocol.gameover == QueryGameover::Both) {
            auto &them = is_p1_turn ? engine2 : engine1;
            them->isready();
            them->position(pos);
            if (them->query_gameover()) {
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

        pos.makemove(movestr);

        // Assume that the side to move alternates with each move
        // If this is not the case, then the "askturn" protocol setting should be set to true
        pos.set_turn(!pos.turn());
    }

    auto result = GameResult::None;
    auto adjudicated = AdjudicationReason::None;

    if (out_of_time) {
        result = pos.turn() == Side::Player1 ? GameResult::Player2Win : GameResult::Player1Win;
        adjudicated = AdjudicationReason::Timeout;
    } else if (gameover_claimed) {
        engine1->isready();
        engine1->position(pos);
        const auto gameover1 = engine1->query_gameover();
        const auto result1 = engine1->query_result();

        engine2->isready();
        engine2->position(pos);
        const auto gameover2 = engine2->query_gameover();
        const auto result2 = engine2->query_result();

        if (gameover1 != gameover2) {
            adjudicated = AdjudicationReason::GameoverMismatch;
        } else if (result1 != result2) {
            adjudicated = AdjudicationReason::ResultMismatch;
        } else {
            result = result1;
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
