#include <doctest/doctest.h>
#include <array>
#include <cmath>
#include <elo.hpp>
#include <tuple>

TEST_CASE("elo") {
    const std::array<std::tuple<int, int, int, float>, 10> tests = {
        std::make_tuple(10, 10, 10, 0.0f),

        std::make_tuple(20, 10, 10, 88.74f),
        std::make_tuple(10, 20, 10, -88.74f),
        std::make_tuple(10, 10, 20, 0.0f),

        std::make_tuple(0, 10, 10, -190.85f),
        std::make_tuple(10, 0, 10, 190.85f),
        std::make_tuple(10, 10, 0, 0.0f),

        std::make_tuple(300, 100, 100, 147.19f),
        std::make_tuple(100, 300, 100, -147.19f),
        std::make_tuple(100, 100, 300, 0.00f),
    };

    for (const auto &[w, l, d, expected] : tests) {
        const auto elo = get_elo(w, l, d);
        const auto rounded = std::round(elo * 100) / 100;
        REQUIRE(rounded == expected);
    }
}

TEST_CASE("err") {
    const std::array<std::tuple<int, int, int, float>, 10> tests = {
        std::make_tuple(10, 10, 10, 104.40f),

        std::make_tuple(20, 10, 10, 98.10f),
        std::make_tuple(10, 20, 10, 98.10f),
        std::make_tuple(10, 10, 20, 77.27f),

        std::make_tuple(0, 10, 10, 107.05f),
        std::make_tuple(10, 0, 10, 107.05f),
        std::make_tuple(10, 10, 0, 163.05f),

        std::make_tuple(300, 100, 100, 29.06f),
        std::make_tuple(100, 300, 100, 29.06f),
        std::make_tuple(100, 100, 300, 19.25f),
    };

    for (const auto &[w, l, d, expected] : tests) {
        const auto err = get_err(w, l, d);
        const auto rounded = std::round(err * 100) / 100;
        REQUIRE(rounded == expected);
    }
}
