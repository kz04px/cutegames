#ifndef SPRT_HPP
#define SPRT_HPP

#include <cmath>
#include <tuple>

namespace {

[[nodiscard]] auto elo_to_probability(const float elo,
                                                const float drawelo) -> std::tuple<float, float, float> {
    const auto pwin = 1.0f / (1.0f + std::pow(10.0f, (-elo + drawelo) / 400.0f));
    const auto ploss = 1.0f / (1.0f + std::pow(10.0f, (elo + drawelo) / 400.0f));
    const auto pdraw = 1.0f - pwin - ploss;
    return {pwin, pdraw, ploss};
}

[[nodiscard]] auto probability_to_elo(const float pwin,
                                                const float,
                                                const float ploss) -> std::pair<float, float> {
    const auto elo = 200.0f * std::log10(pwin / ploss * (1.0f - ploss) / (1.0f - pwin));
    const auto draw_elo = 200.0f * std::log10((1.0f - ploss) / ploss * (1.0f - pwin) / pwin);
    return {elo, draw_elo};
}

}  // namespace

namespace sprt {

[[nodiscard]] auto get_llr(int wins, int losses, int draws, const float elo0, const float elo1) -> float {
    wins = std::max(wins, 1);
    losses = std::max(losses, 1);
    draws = std::max(draws, 1);

    const auto total = wins + losses + draws;

    const auto [elo, drawelo] = probability_to_elo(float(wins) / total, float(draws) / total, float(losses) / total);

    const auto [p0win, p0draw, p0loss] = elo_to_probability(elo0, drawelo);
    const auto [p1win, p1draw, p1loss] = elo_to_probability(elo1, drawelo);

    const auto wins_factor = wins * std::log(p1win / p0win);
    const auto losses_factor = losses * std::log(p1loss / p0loss);
    const auto draws_factor = draws * std::log(p1draw / p0draw);

    return wins_factor + losses_factor + draws_factor;
}

[[nodiscard]] auto get_lbound(const float alpha, const float beta) -> float {
    return std::log(beta / (1.0f - alpha));
}

[[nodiscard]] auto get_ubound(const float alpha, const float beta) -> float {
    return std::log((1.0f - beta) / alpha);
}

[[nodiscard]] auto should_stop(const int wins,
                                         const int losses,
                                         const int draws,
                                         const float elo0,
                                         const float elo1,
                                         const float alpha,
                                         const float beta) -> bool {
    const auto llr = get_llr(wins, losses, draws, elo0, elo1);
    const auto lbound = get_lbound(alpha, beta);
    const auto ubound = get_ubound(alpha, beta);
    return llr <= lbound || llr >= ubound;
}

}  // namespace sprt

#endif
