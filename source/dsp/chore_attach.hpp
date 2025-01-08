// Copyright (C) 2025 - zsliu98
// This file is part of ZLDuckerTest
//
// ZLDuckerTest is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLDuckerTest is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLDuckerTest. If not, see <https://www.gnu.org/licenses/>.

#ifndef CHORE_ATTACH_HPP
#define CHORE_ATTACH_HPP

#include "controller.hpp"
#include "dsp_definitions.hpp"

namespace zlDSP {
    template<typename FloatType>
    class ChoreAttach final : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit ChoreAttach(juce::AudioProcessor &processor,
                             juce::AudioProcessorValueTreeState &parameters,
                             Controller<FloatType> &controller);

        ~ChoreAttach() override;

        void addListeners();

    private:
        juce::AudioProcessor &processorRef;
        juce::AudioProcessorValueTreeState &parameterRef;
        Controller<FloatType> &controllerRef;

        constexpr static std::array IDs{
            sideChain::ID, strength::ID, duckRange::ID
        };
        constexpr static std::array defaultVs{
            static_cast<float>(sideChain::defaultV),
            strength::defaultV, duckRange::defaultV
        };

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void initDefaultValues();
    };
} // zlDSP

#endif //CHORE_ATTACH_HPP
