#pragma once

#include <cmath>
#include <numbers>
#include <limits>

template<typename FloatType>
class WaveShaper {
public:
    WaveShaper() = default;

    FloatType processNormal(FloatType x) {
        return g0(x);
    }

    FloatType processADAA(FloatType x) {
        FloatType d;
        FloatType y{};
        if (std::abs(x - x1) < kEps) {
            d = g1(FloatType(0.5) * (x + x1));
        } else {
            d = (g2(x) - g2(x1)) / (x - x1);
        }
        if (std::abs(x - x0) < kEps) {
            const auto x_bar = FloatType(0.5) * (x + x0);
            const auto delta = x_bar - x1;
            if (std::abs(delta) < kEps) {
                y = g0(FloatType(0.5) * (x_bar + x1));
            } else {
                y = FloatType(2) / delta * (g1(x_bar) + (g2(x1) - g2(x_bar)) / delta);
            }
        } else {
            y = FloatType(2) / (x - x0) * (d - d0);
        }
        x0 = x1;
        x1 = x;
        d0 = d;
        return y;
    }

private:
    FloatType x0, x1, d0;
    static constexpr FloatType kEps = FloatType(1e-10);

    inline FloatType g0(FloatType x) {
        return x > 0
                   ? x * (FloatType(2) - x)
                   : x * (FloatType(2) + x);
    }

    inline FloatType g1(FloatType x) {
        return x > 0
                   ? x * x * (FloatType(1) - x * (FloatType(1) / FloatType(3)))
                   : x * x * (FloatType(1) + x * (FloatType(1) / FloatType(3)));
    }

    inline FloatType g2(FloatType x) {
        return x > 0
                   ? x * x * x * (FloatType(1) / FloatType(3) - x * (FloatType(1) / FloatType(12)))
                   : x * x * x * (FloatType(1) / FloatType(3) + x * (FloatType(1) / FloatType(12)));
    }
};
