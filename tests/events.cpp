#include <doctest/doctest.h>
#include <libevents.hpp>

struct [[nodiscard]] TestEvent : public libevents::Event {
    [[nodiscard]] virtual auto id() const noexcept -> libevents::Event::EventIDType override {
        return 0;
    }
};

struct [[nodiscard]] TestEvent2 : public libevents::Event {
    [[nodiscard]] virtual auto id() const noexcept -> libevents::Event::EventIDType override {
        return 1;
    }
};

TEST_CASE("Dispatcher::empty()") {
    auto dispatcher = libevents::Dispatcher();
    REQUIRE(dispatcher.empty());
    dispatcher.post_event(std::make_shared<TestEvent>());
    REQUIRE(!dispatcher.empty());
}

TEST_CASE("Dispatcher::size()") {
    auto dispatcher = libevents::Dispatcher();
    REQUIRE(dispatcher.size() == 0);
    dispatcher.post_event(std::make_shared<TestEvent>());
    REQUIRE(dispatcher.size() == 1);
    dispatcher.post_event(std::make_shared<TestEvent>());
    dispatcher.post_event(std::make_shared<TestEvent>());
    REQUIRE(dispatcher.size() == 3);
}

TEST_CASE("Dispatcher::clear()") {
    auto dispatcher = libevents::Dispatcher();
    dispatcher.post_event(std::make_shared<TestEvent>());
    dispatcher.post_event(std::make_shared<TestEvent>());
    dispatcher.post_event(std::make_shared<TestEvent>());
    REQUIRE(dispatcher.size() == 3);
    dispatcher.clear();
    REQUIRE(dispatcher.empty());
}

TEST_CASE("Dispatcher::send_all()") {
    int received = 0;
    auto listener = [&received](const auto &) {
        received++;
    };

    auto dispatcher = libevents::Dispatcher();

    // Register listeners
    dispatcher.register_event_listener(0, listener);

    // Create & send events
    dispatcher.post_event(std::make_shared<TestEvent>());
    dispatcher.post_event(std::make_shared<TestEvent>());
    dispatcher.send_all();

    REQUIRE(dispatcher.empty());
    REQUIRE(received == 2);
}

TEST_CASE("Dispatcher::send_event()") {
    int received = 0;
    auto listener = [&received](const auto &) {
        received++;
    };

    auto dispatcher = libevents::Dispatcher();

    // Register listeners
    dispatcher.register_event_listener(0, listener);

    // Create & send events right away
    dispatcher.send_event(std::make_shared<TestEvent>());
    dispatcher.send_event(std::make_shared<TestEvent>());

    REQUIRE(dispatcher.empty());
    REQUIRE(received == 2);
}

TEST_CASE("libevents big boy") {
    int received1 = 0;
    int received2 = 0;
    auto listener1 = [&received1](const auto &) {
        received1++;
    };
    auto listener2 = [&received2](const auto &) {
        received2++;
    };

    auto dispatcher = libevents::Dispatcher();

    // Register listeners
    dispatcher.register_event_listener(0, listener1);
    dispatcher.register_event_listener(1, listener2);

    // Create & send events
    dispatcher.post_event(std::make_shared<TestEvent>());
    dispatcher.post_event(std::make_shared<TestEvent2>());
    dispatcher.post_event(std::make_shared<TestEvent2>());
    dispatcher.send_all();

    REQUIRE(dispatcher.empty());
    REQUIRE(received1 == 1);
    REQUIRE(received2 == 2);
}
