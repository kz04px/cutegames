#ifndef ELO_HPP
#define ELO_HPP

#include <cassert>
#include <cmath>
#include <limits>
#include <numbers>

namespace {

static constexpr auto pi = std::numbers::pi_v<float>;

[[nodiscard]] constexpr auto get_mu(const int w, const int l, const int d) noexcept -> float {
    return (w + (d / 2.0f)) / (w + l + d);
}

[[nodiscard]] auto get_erf_inv(const float x) noexcept -> float {
    const auto a = 8.0f * (pi - 3.0f) / (3.0f * pi * (4.0f - pi));
    const auto y = std::log(1.0f - x * x);
    const auto z = 2.0f / (pi * a) + y / 2.0f;
    const auto ret = std::sqrt(std::sqrt(z * z - y / a) - z);
    if (x >= 0.0f) {
        return ret;
    } else {
        return -ret;
    }
}

[[nodiscard]] auto get_phi_inv(const float p) noexcept -> float {
    return std::sqrt(2.0f) * get_erf_inv(2.0f * p - 1.0f);
}

[[nodiscard]] auto get_diff(const float p) noexcept -> float {
    if (p >= 1.0f) {
        return std::numeric_limits<float>::infinity();
    } else if (p <= 0.0f) {
        return -std::numeric_limits<float>::infinity();
    } else {
        return -400.0f * std::log10(1.0f / p - 1.0f);
    }
}

}  // namespace

[[nodiscard]] auto los(const int w, const int l) noexcept -> float {
    if (w + l == 0) {
        return std::numeric_limits<float>::quiet_NaN();
    }
    return 100.0f * (0.5f + 0.5f * std::erf((w - l) / std::sqrt(2.0f * (w + l))));
}

[[nodiscard]] auto get_elo(const int w, const int l, const int d) noexcept -> float {
    if (w + l + d == 0) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    const auto value = get_diff(get_mu(w, l, d));

    return value == -0.0f ? 0.0f : value;
}

[[nodiscard]] auto get_err(const int w, const int l, const int d) noexcept -> float {
    const auto total = w + l + d;

    if (total == 0) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    const auto mu = get_mu(w, l, d);

    const auto w_ratio = static_cast<float>(w) / total;
    const auto l_ratio = static_cast<float>(l) / total;
    const auto d_ratio = static_cast<float>(d) / total;

    const auto devW = w_ratio * std::pow(1.0f - mu, 2.0f);
    const auto devL = l_ratio * std::pow(0.0f - mu, 2.0f);
    const auto devD = d_ratio * std::pow(0.5f - mu, 2.0f);

    const auto m_stdev = std::sqrt(devW + devL + devD) / std::sqrt(static_cast<float>(total));

    const auto muMin = mu + get_phi_inv(0.025f) * m_stdev;
    const auto muMax = mu + get_phi_inv(0.975f) * m_stdev;

    return (get_diff(muMax) - get_diff(muMin)) / 2.0f;
}

#endif
