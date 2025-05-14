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

    FloatType processADAA(FloatType x2) {
        FloatType y{};
        const auto g2_x2 = g2(x2);
        const auto x2_x1_delta = x2 - x1_;
        FloatType d1 = std::abs(x2_x1_delta) > kEps
                           ? (g2_x2 - g2_x1_) / (x2_x1_delta) // x2 - x1 normal condition
                           : g1(FloatType(0.5) * (x2 + x1_)); // x2 - x1 ill condition
        const auto x2_x0_delta = x2 - x0_;
        if (std::abs(x2_x0_delta) > kEps) { // x2 - x0 normal condition
            y = FloatType(2) / (x2_x0_delta) * (d1 - d0_);
        } else { // x2 - x0 ill condition
            const auto x_bar = FloatType(0.5) * (x2 + x0_);
            const auto delta = x_bar - x1_;
            if (std::abs(delta) > kEps) { // delta normal condition
                const auto delta_r = FloatType(1) / delta;
                y = FloatType(2) * delta_r * (g1(x_bar) + (g2_x1_ - g2(x_bar)) * delta_r);
            } else { // delta ill condition
                y = g0(FloatType(0.5) * (x_bar + x1_));
            }
        }
        // save states
        x0_ = x1_;
        x1_ = x2;
        d0_ = d1;
        g2_x1_ = g2_x2;
        return y;
    }

private:
    FloatType x0_{}, x1_{}, d0_{}, g2_x1_{};
    static constexpr FloatType kEps = FloatType(1e-15);

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
