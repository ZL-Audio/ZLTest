// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef STATIC_GAIN_COMPENSATION_HPP
#define STATIC_GAIN_COMPENSATION_HPP

#include "single_filter.hpp"

namespace zlIIR {
    template<typename FloatType>
    class StaticGainCompensation {
    public:
        explicit StaticGainCompensation(Filter<FloatType> &filter);

        void prepare(const juce::dsp::ProcessSpec &spec);

        void update();

        void process(juce::AudioBuffer<FloatType> &buffer);

        void process(juce::dsp::AudioBlock<FloatType> block);

        void enable(const bool f) {
            isON.store(f);
            if (f) toUpdate.store(true);
        }

    private:
        Filter<FloatType> &target;
        juce::dsp::Gain<FloatType> gain;
        std::atomic<bool> isON{false}, toUpdate{false};

        static inline FloatType integrateFQ(const FloatType f1, const FloatType f2) {
            const auto w1 = FloatType(1.0000057078597646) + FloatType(1.3450513160225395 * 1e-8) * f1 * f1;
            const auto w2 = FloatType(1.0000057078597646) + FloatType(1.3450513160225395 * 1e-8) * f2 * f2;
            return static_cast<FloatType>(std::log((w1 + 1) * (1 - w2) / (w2 + 1) / (1 - w1)));
        }

        static constexpr FloatType k1 = FloatType(0.165602);
        static constexpr FloatType k2 = FloatType(0.338973);
        static constexpr FloatType k3 = FloatType(0.712232);
        static constexpr FloatType k4 = FloatType(0.374335);
        static constexpr FloatType k5 = FloatType(1.494580);
        static constexpr FloatType k6 = FloatType(7.131157);
        static constexpr FloatType k7 = FloatType(0.014366);
        static constexpr std::array<FloatType, 5> ls{
            FloatType(0.0715434186421697),
            FloatType(-0.1464669007001578),
            FloatType(0.3387747583736678),
            FloatType(1.6910475603407424),
            FloatType(0.0119044419409884),
        };

        static constexpr std::array<FloatType, 5> hs{
            FloatType(-0.1479456582403618),
            FloatType(0.2781951365993936),
            FloatType(0.3329719197080591),
            FloatType(4.4029889648382801),
            FloatType(0.0006745841678141),
        };

        static FloatType getPeakEstimation(const FloatType f, const FloatType g, const FloatType q) {
            const auto bw = static_cast<FloatType>(std::asinh(0.5 / q) / std::log(2));
            const auto scale = static_cast<FloatType>(std::pow(2, bw / 2));
            const auto f1 = juce::jlimit(FloatType(10), FloatType(20000), f / scale);
            const auto f2 = juce::jlimit(FloatType(10), FloatType(20000), f * scale);
            const auto fqEffect = integrateFQ(f1, f2);
            auto res = k1 * std::pow(fqEffect + k2 * bw, k3) * g * (
                           k4 / (std::pow(bw, k5) + k6) * g * (k7 * g + 1) + 1);
            if (g > 0) {
                res = std::max(FloatType(0), res);
            } else {
                res = std::min(FloatType(0), res);
            }
            return -res;
        }

        static FloatType getLowShelfEstimation(FloatType f, const FloatType g) {
            f = juce::jlimit(FloatType(15), FloatType(5000), f);
            const auto bw = static_cast<FloatType>(std::log2(f / FloatType(10)));
            const auto fqEffect = integrateFQ(10, f);
            auto res = ls[0] * (fqEffect + ls[1] * bw) * g * (ls[2] / (bw + ls[3]) * g * (ls[4] * g + 1) + 1);
            if (g > 0) {
                res = std::max(FloatType(0), res);
            } else {
                res = std::min(FloatType(0), res);
            }
            return -res;
        }

        static FloatType getHighShelfEstimation(FloatType f, const FloatType g) {
            f = juce::jlimit(FloatType(200), FloatType(16000), f);
            const auto bw = static_cast<FloatType>(std::log2(FloatType(20000) / f));
            const auto fqEffect = integrateFQ(f, 20000);
            auto res = (hs[0] * fqEffect + hs[1] * bw) * g * (hs[2] / (bw + hs[3]) * g * (hs[4] * g + 1) + 1);
            if (g > 0) {
                res = std::max(FloatType(0), res);
            } else {
                res = std::min(FloatType(0), res);
            }
            return -res;
        }
    };
} // zlIIR

#endif //STATIC_GAIN_COMPENSATION_HPP
