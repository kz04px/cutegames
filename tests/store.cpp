#include <doctest/doctest.h>
#include <store.hpp>
#include <utils.hpp>

TEST_CASE("Store::release()") {
    auto store = Store<int>(4);

    REQUIRE(!store.release(std::make_shared<int>()));
    REQUIRE(!store.release(std::make_shared<int>()));
    REQUIRE(!store.release(std::make_shared<int>()));
    REQUIRE(!store.release(std::make_shared<int>()));
    REQUIRE(store.release(std::make_shared<int>()));
}

TEST_CASE("Store::get()") {
    auto store = Store<int>(4);

    store.release(std::make_shared<int>(1));
    store.release(std::make_shared<int>(2));

    const auto a = store.get([](const auto obj) {
        return *obj == 1;
    });
    REQUIRE(a);
    REQUIRE(**a == 1);

    const auto b = store.get([](const auto obj) {
        return *obj == 3;
    });
    REQUIRE(!b);

    const auto c = store.get();
    REQUIRE(c);
    REQUIRE(**c == 2);

    const auto d = store.get();
    REQUIRE(!d);
}

TEST_CASE("Store::empty()") {
    auto store = Store<int>(4);
    REQUIRE(store.empty());
    store.release(std::make_shared<int>());
    REQUIRE(!store.empty());
}

TEST_CASE("Store::full()") {
    auto store = Store<int>(4);
    REQUIRE(!store.full());
    store.release(std::make_shared<int>());
    store.release(std::make_shared<int>());
    store.release(std::make_shared<int>());
    store.release(std::make_shared<int>());
    REQUIRE(store.full());
}

TEST_CASE("Store::capacity()") {
    auto store = Store<int>(4);
    REQUIRE(store.capacity() == 4);
}

TEST_CASE("Store::size()") {
    auto store = Store<int>(4);
    REQUIRE(store.size() == 0);
    store.release(std::make_shared<int>());
    REQUIRE(store.size() == 1);
    store.release(std::make_shared<int>());
    REQUIRE(store.size() == 2);
    store.release(std::make_shared<int>());
    REQUIRE(store.size() == 3);
    store.release(std::make_shared<int>());
    REQUIRE(store.size() == 4);
    store.release(std::make_shared<int>());
    REQUIRE(store.size() == 4);
    store.release(std::make_shared<int>());
    REQUIRE(store.size() == 4);
}

TEST_CASE("Store::clear()") {
    auto store = Store<int>(4);
    store.release(std::make_shared<int>());
    store.release(std::make_shared<int>());
    store.release(std::make_shared<int>());
    REQUIRE(store.size() == 3);
    store.clear();
    REQUIRE(store.size() == 0);
}

TEST_CASE("YEE") {
    const std::pair<std::string, std::string> tests[] = {
        {"", ""},
        {"nospace", "nospace"},
        {"There are four lights!", "There are four lights!"},
        {"double  space", "double space"},
        {"trailing space ", "trailing space"},
        {" leading space", "leading space"},
        {" ", ""},
        {"  ", ""},
        {"     ", ""},
    };

    auto combine = [](const std::vector<std::string_view> &parts, const char spacer = ' ') {
        std::string result;

        bool first = true;
        for (const auto part : parts) {
            if (!first) {
                result += spacer;
            }
            result += part;
            first = false;
        }

        return result;
    };

    for (const auto &[input, expected] : tests) {
        const auto parts = utils::split(input, " ");
        const auto combined = combine(parts);
        REQUIRE(combined == expected);
    }
}
