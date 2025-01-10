// Copyright (C) 2025 - zsliu98
// This file is part of ZLDuckerTest
//
// ZLDuckerTest is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLDuckerTest is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLDuckerTest. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLDUCKER_CONTROLLER_HPP
#define ZLDUCKER_CONTROLLER_HPP

#include "dsp_definitions.hpp"
#include "filter/filter.hpp"
#include "container/container.hpp"

namespace zlDSP {
    template<typename FloatType>
    class Controller final {
    public:
        static constexpr size_t FilterNum = 16;

        static constexpr std::array<double, 16> duckerPos = {
            1.25051561e+01, 2.01471012e+01, 3.24590660e+01, 5.22949160e+01,
            8.42525240e+01, 1.35739539e+02, 2.18690452e+02, 3.52332960e+02,
            5.67644877e+02, 9.14534669e+02, 1.47341004e+03, 2.37381612e+03,
            3.82446355e+03, 6.16160674e+03, 9.92698641e+03, 1.59934029e+04
        };

        explicit Controller(juce::AudioProcessor &processor);

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<FloatType> &buffer);

        void setStrength(const FloatType x) {
            if (x < FloatType(0.5)) {
                strength.store(std::sqrt(x / FloatType(2)));
            } else {
                strength.store(FloatType(0.2) * x + FloatType(0.4));
            }
        }

        void setDuckRange(const FloatType x) { range.store(x); }

        void setFocus(const FloatType x) {
            focus.store(x);
            toUpdateFocus.store(true);
        }

        void setIsSide(const bool x) { useSide.store(x); }

    private:
        juce::AudioProcessor &processorRef;
        juce::AudioBuffer<FloatType> passBuffer{};
        juce::AudioBuffer<FloatType> sampleBuffer;

        std::array<FloatType, FilterNum> mainPassRMS{}, auxPassRMS{};
        std::array<FloatType, FilterNum> currentGains{}, deltaGains{};
        zlContainer::FixedMaxSizeArray<size_t, FilterNum> dynamicIndices{}, staticIndices{};
        std::array<zlFilter::SmoothIIR<FloatType, 1, false, false, false>, FilterNum> duckerFilters;
        std::array<zlFilter::SmoothIIR<FloatType, 1, false, false, false>, FilterNum> mainPassFilters;
        std::array<zlFilter::SmoothIIR<FloatType, 1, false, false, false>, FilterNum> auxPassFilters;

        std::atomic<bool> useSide{false};
        bool currentUseSide{false};

        int samplePerBuffer{1};

        std::atomic<FloatType> strength{FloatType(0.5)}, range{FloatType(10)}, focus{FloatType(0.5)};
        FloatType currentStrength{FloatType(0.5)}, currentRange{FloatType(10)};

        std::atomic<bool> toUpdateFocus{true};

        void processSubBuffer(juce::AudioBuffer<FloatType> &buffer);

        // juce::FileLogger logger{juce::File{"/Volumes/Ramdisk/log.txt"}, ""};
    };
} // zlDSP

#endif //ZLDUCKER_CONTROLLER_HPP
