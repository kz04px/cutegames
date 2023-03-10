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
    auto gameresult = GameResult::None;
    auto tc = settings.timecontrol;
    auto out_of_time = false;

    // Find out whose turn it is
    engine1->position(pos);
    pos.set_turn(engine1->query_p1turn() ? Side::Player1 : Side::Player2);

    // Play a game
    while (true) {
        const auto is_p1_turn = pos.turn() == Side::Player1;
        auto &us = is_p1_turn ? engine1 : engine2;

        // Inform the engine of the current position
        us->isready();
        us->position(pos);

        const auto t0 = std::chrono::steady_clock::now();

        // Get move string
        const auto movestr = us->go(tc);

        const auto t1 = std::chrono::steady_clock::now();
        const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);

        pos.makemove(movestr);

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
                if (dt.count() > tc.movetime + settings.timeoutbuffer.count()) {
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

        const auto is_gameover = us->query_gameover();

        if (out_of_time) {
            gameresult = is_p1_turn ? GameResult::Player2Win : GameResult::Player1Win;
            break;
        } else if (is_gameover) {
            us->position(pos);
            gameresult = us->query_result();
            break;
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

    dispatcher.post_event(std::make_shared<GameFinished>(game_id, engine1_id, engine2_id, gameresult, out_of_time));
}
