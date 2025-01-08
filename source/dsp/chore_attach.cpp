// Copyright (C) 2025 - zsliu98
// This file is part of ZLDuckerTest
//
// ZLDuckerTest is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLDuckerTest is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLDuckerTest. If not, see <https://www.gnu.org/licenses/>.

#include "chore_attach.hpp"

namespace zlDSP {
    template<typename FloatType>
    ChoreAttach<FloatType>::ChoreAttach(juce::AudioProcessor &processor,
                                        juce::AudioProcessorValueTreeState &parameters,
                                        Controller<FloatType> &controller)
        : processorRef(processor),
          parameterRef(parameters),
          controllerRef(controller) {
        for (auto &ID: IDs) {
            parameterRef.addParameterListener(ID, this);
        }
        for (size_t j = 0; j < defaultVs.size(); ++j) {
            parameterChanged(IDs[j], defaultVs[j]);
        }
    }

    template<typename FloatType>
    ChoreAttach<FloatType>::~ChoreAttach() {
        for (auto &ID: IDs) {
            parameterRef.removeParameterListener(ID, this);
        }
    }

    template<typename FloatType>
    void ChoreAttach<FloatType>::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == sideChain::ID) {
            controllerRef.setIsSide(newValue > 0.5f);
        } else if (parameterID == strength::ID) {
            controllerRef.setStrength(static_cast<FloatType>(newValue));
        } else if (parameterID == duckRange::ID) {
            controllerRef.setDuckRange(static_cast<FloatType>(newValue));
        }
    }

    template
    class ChoreAttach<float>;

    template
    class ChoreAttach<double>;
} // zlDSP
