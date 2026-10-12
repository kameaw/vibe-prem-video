#pragma once

#include <cstdint>
#include <compare>

namespace vibe {

struct Rational
{
    int32_t num = 0;
    int32_t den = 1;

    constexpr Rational() = default;
    constexpr Rational(int32_t n, int32_t d) : num(n), den(d == 0 ? 1 : d) {}

    [[nodiscard]] constexpr double toDouble() const
    {
        return den != 0 ? static_cast<double>(num) / static_cast<double>(den) : 0.0;
    }

    [[nodiscard]] constexpr bool isValid() const { return den != 0 && num != 0; }

    constexpr auto operator<=>(const Rational&) const = default;
};

/// Editorial / media time as integer ticks in a rational rate.
struct Time
{
    int64_t ticks = 0;
    Rational rate{1, 1};

    [[nodiscard]] static Time fromSeconds(double seconds, Rational rate)
    {
        Time t;
        t.rate = rate;
        if (rate.den == 0) {
            return t;
        }
        t.ticks = static_cast<int64_t>(seconds * static_cast<double>(rate.num) / static_cast<double>(rate.den)
                                       + (seconds >= 0 ? 0.5 : -0.5));
        return t;
    }

    [[nodiscard]] static Time fromMicroseconds(int64_t us, Rational rate)
    {
        return fromSeconds(static_cast<double>(us) / 1'000'000.0, rate);
    }

    [[nodiscard]] double toSeconds() const
    {
        if (rate.num == 0) {
            return 0.0;
        }
        return static_cast<double>(ticks) * static_cast<double>(rate.den) / static_cast<double>(rate.num);
    }

    [[nodiscard]] int64_t toMicroseconds() const
    {
        return static_cast<int64_t>(toSeconds() * 1'000'000.0 + (ticks >= 0 ? 0.5 : -0.5));
    }
};

} // namespace vibe
