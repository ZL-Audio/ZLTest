//
// Created by Zishu Liu on 12/28/23.
//

#include "two_value_rotary_slider_component.h"

namespace zlInterface {
    TwoValueRotarySliderComponent::TwoValueRotarySliderComponent(const juce::String &labelText, UIBase &base)
        : uiBase(base), slider1LAF(base), slider2LAF(base), nameLookAndFeel(base) {
        for (auto const s: {&slider1, &slider2}) {
            s->setSliderStyle(juce::Slider::Rotary);
            s->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
            s->setDoubleClickReturnValue(true, 0.0);
            s->setScrollWheelEnabled(true);
            s->setInterceptsMouseClicks(false, false);
        }
        slider1.setLookAndFeel(&slider1LAF);
        slider2LAF.setEditable(showSlider2.load());
        slider2.setLookAndFeel(&slider2LAF);

        addAndMakeVisible(slider1);
        addAndMakeVisible(slider2);

        label.setText(labelText, juce::dontSendNotification);
        nameLookAndFeel.setFontScale(FontHuge);
        label.setLookAndFeel(&nameLookAndFeel);
        label.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(label);

        setInterceptsMouseClicks(true, false);
    }

    TwoValueRotarySliderComponent::~TwoValueRotarySliderComponent() {
        slider1.setLookAndFeel(nullptr);
        slider2.setLookAndFeel(nullptr);
        label.setLookAndFeel(nullptr);
    }

    void TwoValueRotarySliderComponent::paintOverChildren(juce::Graphics &g) {
        if (mouseOver.load()) {
            auto localBound = getLocalBounds().toFloat();
            if (editable.load()) {
                g.setColour(uiBase.getTextColor());
            } else {
                g.setColour(uiBase.getTextInactiveColor());
            }
            if (showSlider2.load()) {
                g.setFont(uiBase.getFontSize() * FontNormal);
                auto value1Bound = localBound.withSizeKeepingCentre(localBound.getWidth() * 0.7f,
                                                                    localBound.getHeight() * 0.6f);
                auto value2Bound = value1Bound.removeFromBottom(value1Bound.getHeight() * 0.5f);
                g.drawText(getDisplayValue(slider1.getValue()), value1Bound, juce::Justification::centredBottom);
                g.drawText(getDisplayValue(slider2.getValue()), value2Bound, juce::Justification::centredTop);
            } else {
                g.setFont(uiBase.getFontSize() * FontHuge);
                g.drawText(getDisplayValue(slider1.getValue()), localBound, juce::Justification::centred);
            }
        }
    }

    juce::String TwoValueRotarySliderComponent::getDisplayValue(double value) {

        juce::String labelToDisplay = juce::String(slider1.getTextFromValue(value)).substring(0, 4);
        if (value < 10000 && labelToDisplay.contains(".")) {
            labelToDisplay = juce::String(value).substring(0, 5);
        }
        if (value > 10000) {
            value = value / 1000;
            labelToDisplay = juce::String(value).substring(0, 4) + "K";
        }
        return labelToDisplay;
    }

    void TwoValueRotarySliderComponent::mouseUp(const juce::MouseEvent &event) {
        if (!showSlider2.load() || (event.mods.isLeftButtonDown() && !event.mods.isCommandDown())) {
            slider1.mouseUp(event);
        } else {
            slider2.mouseUp(event);
        }
    }

    void TwoValueRotarySliderComponent::mouseDown(const juce::MouseEvent &event) {
        if (!showSlider2.load() || (event.mods.isLeftButtonDown() && !event.mods.isCommandDown())) {
            slider1.mouseDown(event);
        } else {
            slider2.mouseDown(event);
        }
    }

    void TwoValueRotarySliderComponent::mouseDrag(const juce::MouseEvent &event) {
        if (!showSlider2.load() || (event.mods.isLeftButtonDown() && !event.mods.isCommandDown())) {
            slider1.mouseDrag(event);
        } else {
            slider2.mouseDrag(event);
        }
    }

    void TwoValueRotarySliderComponent::mouseEnter(const juce::MouseEvent &event) {
        slider1.mouseEnter(event);
        slider2.mouseEnter(event);
        mouseOver.store(true);
        label.setVisible(false);
        repaint();
    }

    void TwoValueRotarySliderComponent::mouseExit(const juce::MouseEvent &event) {
        slider1.mouseExit(event);
        slider2.mouseExit(event);
        mouseOver.store(false);
        label.setVisible(true);
        repaint();
    }

    void TwoValueRotarySliderComponent::mouseMove(const juce::MouseEvent &event) {
    }

    void TwoValueRotarySliderComponent::mouseDoubleClick(const juce::MouseEvent &event) {
        if (!showSlider2.load() || (event.mods.isLeftButtonDown() && !event.mods.isCommandDown())) {
            slider1.mouseDoubleClick(event);
        } else {
            slider2.mouseDoubleClick(event);
        }
    }

    void TwoValueRotarySliderComponent::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) {
        if (!showSlider2.load() || !event.mods.isCommandDown()) {
            slider1.mouseWheelMove(event, wheel);
        } else {
            slider2.mouseWheelMove(event, wheel);
        }
    }


    void TwoValueRotarySliderComponent::resized() {
        slider1.setBounds(getLocalBounds());
        slider2.setBounds(getLocalBounds());

        auto localBound = getLocalBounds().toFloat();
        auto labelBound = localBound.withSizeKeepingCentre(localBound.getWidth() * 0.7f,
                                                            localBound.getHeight() * 0.6f);
        label.setBounds(labelBound.toNearestInt());
    }
}
