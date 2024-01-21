// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLTest_DRAGGER_COMPONENT_HPP
#define ZLTest_DRAGGER_COMPONENT_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../interface_definitions.hpp"
#include "dragger_look_and_feel.hpp"

namespace zlInterface {
    class Dragger final : public juce::Component {
    public:
        explicit Dragger(UIBase &base);

        ~Dragger() override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;

        void resized() override;

        juce::Slider &getWheelSlider() {return wheelS;}

        void setXPortion(float x);

        void setYPortion(float y);

        float getXPortion() const;

        float getYPortion() const;

        class Listener {
        public:
            virtual ~Listener() = default;

            virtual void draggerValueChanged(Dragger *dragger) = 0;

            virtual void dragStarted(Dragger *dragger) = 0;

            virtual void dragEnded(Dragger *dragger) = 0;
        };

        /** Adds a listener to be called when this slider's value changes. */
        void addListener(Listener *listener);

        /** Removes a previously-registered listener. */
        void removeListener(Listener *listener);

        void setPadding(const float left, const float right,
            const float uppper, const float bottom) {
            lPadding = left;
            rPadding = right;
            uPadding = uppper;
            bPadding = bottom;
        }

    private:
        UIBase &uiBase;

        juce::ToggleButton button;
        DraggerLookAndFeel draggerLAF;
        juce::ComponentDragger dragger;
        juce::ComponentBoundsConstrainer constrainer;
        std::atomic<bool> isSelected;
        std::atomic<float> xPortion, yPortion;

        juce::Rectangle<float> buttonArea;
        float lPadding, rPadding, uPadding, bPadding;

        juce::Slider wheelS;

        juce::ListenerList<Listener> listeners;
    };
} // zlInterface

#endif //ZLTest_DRAGGER_COMPONENT_HPP
