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
    };
} // zlIIR

#endif //STATIC_GAIN_COMPENSATION_HPP
