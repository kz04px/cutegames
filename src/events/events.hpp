#ifndef CUTEGAMES_EVENTS_HPP
#define CUTEGAMES_EVENTS_HPP

#include <chrono>
#include <games/game.hpp>
#include <libevents.hpp>
#include <string>
#include <thread>

enum EventID : libevents::Event::EventIDType
{
    // Game
    zGameStarted = 0,
    zGameFinished,
    // Move
    zMovePlayed,
    zMovePlayedIllegal,
    // Engine
    zEngineLoaded,
    zEngineUnloaded,
    zEngineCrashed,
    // Match
    zMatchFinished,
    // Threads
    zThreadFinished,
};

struct GameStarted : public libevents::Event {
    [[nodiscard]] GameStarted(const int i, const std::string &f, const int id1, const int id2)
        : game_num(i), fen(f), engine1_id(id1), engine2_id(id2) {
    }

    [[nodiscard]] virtual auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zGameStarted;
    }

    int game_num = 0;
    std::string fen;
    int engine1_id = 0;
    int engine2_id = 0;
};

struct GameFinished : public libevents::Event {
    [[nodiscard]] GameFinished(const int i, const int id1, const int id2, const GameResult r, const bool timeloss)
        : game_num(i), engine1_id(id1), engine2_id(id2), result(r), out_of_time(timeloss) {
    }

    [[nodiscard]] virtual auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zGameFinished;
    }

    int game_num = 0;
    int engine1_id = 0;
    int engine2_id = 0;
    GameResult result = GameResult::None;
    bool out_of_time = false;
};

struct MovePlayed : public libevents::Event {
    [[nodiscard]] MovePlayed(const std::string movestr, const std::chrono::milliseconds t) : move(movestr), ms(t) {
    }

    [[nodiscard]] virtual auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zMovePlayed;
    }

    std::shared_ptr<Game> game;
    std::string move;
    std::chrono::milliseconds ms = std::chrono::milliseconds(0);
};

struct MatchFinished : public libevents::Event {
    [[nodiscard]] MatchFinished() = default;

    [[nodiscard]] virtual auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zMatchFinished;
    }
};

struct ThreadFinished : public libevents::Event {
    [[nodiscard]] ThreadFinished(const std::thread::id gg) : thread_id(gg) {
    }

    [[nodiscard]] virtual auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zThreadFinished;
    }

    std::thread::id thread_id;
};

struct EngineCreated : public libevents::Event {
    [[nodiscard]] EngineCreated(const std::size_t a, const std::string &b, const std::string &c)
        : engine_id(a), path(b), name(c) {
    }

    [[nodiscard]] virtual auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zEngineLoaded;
    }

    std::size_t engine_id = 0;
    std::string path;
    std::string name;
};

struct EngineDestroyed : public libevents::Event {
    [[nodiscard]] EngineDestroyed(const std::size_t a, const std::string &b, const std::string &c)
        : engine_id(a), path(b), name(c) {
    }

    [[nodiscard]] virtual auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zEngineUnloaded;
    }

    std::size_t engine_id = 0;
    std::string path;
    std::string name;
};

#endif
