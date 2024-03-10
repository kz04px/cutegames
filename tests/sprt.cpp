#include <doctest/doctest.h>
#include <array>
#include <cmath>
#include <sprt.hpp>
#include <tuple>

TEST_CASE("sprt::llr") {
    const std::array<std::tuple<int, int, int, int, int, float>, 6> tests = {
        std::make_tuple(10, 10, 10, -10, 10, 0.0f),
        std::make_tuple(3415, 3270, 5763, -1, 4, 2.16f),
        std::make_tuple(4413, 4218, 7481, -1, 4, 2.96f),
        std::make_tuple(1382, 1415, 2627, 0, 5, -1.34f),
        std::make_tuple(7238, 7273, 18473, 0, 4, -2.97f),
        std::make_tuple(7446, 7503, 14227, -3, 1, 0.12f),
    };

    for (const auto &[w, l, d, elo0, elo1, expected] : tests) {
        const auto llr = sprt::get_llr(w, l, d, elo0, elo1);
        const auto rounded = std::round(llr * 100) / 100;
        REQUIRE(rounded == expected);
    }
}

TEST_CASE("sprt::lbound") {
    const std::array<std::tuple<float, float, float>, 2> tests = {
        std::make_tuple(0.05f, 0.05f, -2.94f),
        std::make_tuple(0.01f, 0.01f, -4.60f),
    };

    for (const auto &[alpha, beta, expected] : tests) {
        const auto lbound = sprt::get_lbound(alpha, beta);
        const auto rounded = std::round(lbound * 100) / 100;
        CHECK(rounded == expected);
    }
}

TEST_CASE("sprt::ubound") {
    const std::array<std::tuple<float, float, float>, 2> tests = {
        std::make_tuple(0.05f, 0.05f, 2.94f),
        std::make_tuple(0.01f, 0.01f, 4.60f),
    };

    for (const auto &[alpha, beta, expected] : tests) {
        const auto ubound = sprt::get_ubound(alpha, beta);
        const auto rounded = std::round(ubound * 100) / 100;
        CHECK(rounded == expected);
    }
}
