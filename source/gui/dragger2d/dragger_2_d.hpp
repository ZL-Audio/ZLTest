// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLTest_DRAGGER_2_D_HPP
#define ZLTest_DRAGGER_2_D_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../interface_definitions.hpp"
#include "dragger_look_and_feel.hpp"

namespace zlInterface {
    class Dragger2D final : public juce::Component, private juce::Slider::Listener {
    public:
        explicit Dragger2D(UIBase &base);

        ~Dragger2D() override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;

        void resized() override;

        inline juce::Slider &getHorizonS() { return horizonS; }

        inline juce::Slider &getVerticalS() { return verticalS; }

    private:
        UIBase &uiBase;

        juce::ToggleButton button;
        DraggerLookAndFeel draggerLAF;
        juce::ComponentDragger dragger;
        juce::ComponentBoundsConstrainer constrainer;
        std::atomic<bool> isSelected;

        juce::Slider horizonS, verticalS, wheelS;

        void sliderValueChanged(juce::Slider *slider) override;
    };
} // zlInterface

#endif //ZLTest_DRAGGER_2_D_HPP
