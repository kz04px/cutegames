#ifndef CUTEGAMES_EVENTS_HPP
#define CUTEGAMES_EVENTS_HPP

#include <chrono>
#include <libevents.hpp>
#include <string>
#include <thread>
#include <utility>
#include "../games/game.hpp"

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

struct [[nodiscard]] GameStarted final : public libevents::Event {
    [[nodiscard]] GameStarted(const int i, std::string f, const int id1, const int id2)
        : game_num(i), fen(std::move(f)), engine1_id(id1), engine2_id(id2) {
    }

    [[nodiscard]] auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zGameStarted;
    }

    int game_num = 0;
    std::string fen;
    int engine1_id = 0;
    int engine2_id = 0;
};

struct [[nodiscard]] GameFinished final : public libevents::Event {
    [[nodiscard]] GameFinished(const int i,
                               const int id1,
                               const int id2,
                               const GameResult r,
                               const AdjudicationReason gg,
                               const std::shared_ptr<Game> g)
        : game_num(i), engine1_id(id1), engine2_id(id2), result(r), reason(gg), game(g) {
    }

    [[nodiscard]] auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zGameFinished;
    }

    int game_num = 0;
    int engine1_id = 0;
    int engine2_id = 0;
    GameResult result = GameResult::None;
    AdjudicationReason reason = AdjudicationReason::None;
    std::shared_ptr<Game> game;
};

struct [[nodiscard]] MovePlayed final : public libevents::Event {
    [[nodiscard]] MovePlayed(std::string movestr, const std::chrono::milliseconds t) : move(std::move(movestr)), ms(t) {
    }

    [[nodiscard]] auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zMovePlayed;
    }

    std::shared_ptr<Game> game;
    std::string move;
    std::chrono::milliseconds ms = std::chrono::milliseconds(0);
};

struct [[nodiscard]] MatchFinished final : public libevents::Event {
    [[nodiscard]] MatchFinished() = default;

    [[nodiscard]] auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zMatchFinished;
    }
};

struct [[nodiscard]] ThreadFinished final : public libevents::Event {
    [[nodiscard]] explicit ThreadFinished(const std::thread::id gg) : thread_id(gg) {
    }

    [[nodiscard]] auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zThreadFinished;
    }

    std::thread::id thread_id;
};

struct [[nodiscard]] EngineCreated final : public libevents::Event {
    [[nodiscard]] EngineCreated(const std::size_t a, std::string b, std::string c)
        : engine_id(a), path(std::move(b)), name(std::move(c)) {
    }

    [[nodiscard]] auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zEngineLoaded;
    }

    std::size_t engine_id = 0;
    std::string path;
    std::string name;
};

struct [[nodiscard]] EngineDestroyed final : public libevents::Event {
    [[nodiscard]] EngineDestroyed(const std::size_t a, std::string b, std::string c)
        : engine_id(a), path(std::move(b)), name(std::move(c)) {
    }

    [[nodiscard]] auto id() const noexcept -> libevents::Event::EventIDType override {
        return EventID::zEngineUnloaded;
    }

    std::size_t engine_id = 0;
    std::string path;
    std::string name;
};

#endif
