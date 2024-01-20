// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "dragger_2_d.hpp"

namespace zlInterface {
    Dragger2D::Dragger2D(UIBase &base)
        : uiBase(base), draggerLAF(base) {
        horizonS.addListener(this);
        verticalS.addListener(this);
        wheelS.addListener(this);
        for (auto &s: {&horizonS, &verticalS, &wheelS}) {
            s->setSliderSnapsToMousePosition(false);
            s->setInterceptsMouseClicks(false, false);
            s->setDoubleClickReturnValue(false, 0.0);
        }
        horizonS.setSliderStyle(juce::Slider::LinearHorizontal);
        verticalS.setSliderStyle(juce::Slider::LinearVertical);
        wheelS.setSliderStyle(juce::Slider::LinearHorizontal);

        // juce::Path circle;
        // circle.addEllipse(.1f, .1f, .8f, .8f);
        button.addMouseListener(this, false);
        draggerLAF.setColour(uiBase.getColorMap1(1));
        button.setClickingTogglesState(false);
        button.setLookAndFeel(&draggerLAF);
        addAndMakeVisible(button);
    }

    Dragger2D::~Dragger2D() {
        horizonS.removeListener(this);
        verticalS.removeListener(this);
        wheelS.removeListener(this);

        button.removeMouseListener(this);
        button.setLookAndFeel(nullptr);
    }

    void Dragger2D::mouseDown(const juce::MouseEvent &event) {
        if (button.contains(event.getPosition())) {
            isSelected.store(true);
            button.setToggleState(true, juce::NotificationType::sendNotificationSync);
            dragger.startDraggingComponent(&button, event);
        } else {
            button.setToggleState(false, juce::NotificationType::sendNotificationSync);
        }
    }

    void Dragger2D::mouseUp(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        isSelected.store(false);
    }

    void Dragger2D::mouseDrag(const juce::MouseEvent &event) {
        if (isSelected.load()) {
            dragger.dragComponent(&button, event, &constrainer);
            const auto bound = getLocalBounds().toFloat();
            const auto horizonP = (button.getBoundsInParent().toFloat().getCentreX() - bound.getX()) / bound.getWidth();
            const auto verticalP = 1 - (button.getBoundsInParent().toFloat().getCentreY() - bound.getY()) / bound.getHeight();
            horizonS.setValue(horizonS.getNormalisableRange().convertFrom0to1(static_cast<double>(horizonP)));
            verticalS.setValue(verticalS.getNormalisableRange().convertFrom0to1(static_cast<double>(verticalP)));
        }
    }

    void Dragger2D::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) {
        if (isSelected.load()) {
            wheelS.mouseWheelMove(event, wheel);
        }
    }

    void Dragger2D::resized() {
        auto bound = getLocalBounds().toFloat();
        horizonS.setBounds(bound.withSizeKeepingCentre(bound.getWidth(),
                                                       bound.getHeight()).toNearestInt());
        verticalS.setBounds(bound.withSizeKeepingCentre(bound.getWidth(),
                                                        bound.getHeight()).toNearestInt());
        sliderValueChanged(nullptr);
        bound = bound.withSizeKeepingCentre(bound.getWidth() - uiBase.getFontSize(),
                                            bound.getHeight() - uiBase.getFontSize());
        bound = bound.withSizeKeepingCentre(uiBase.getFontSize(), uiBase.getFontSize());
        button.setBounds(bound.toNearestInt());
        // set constrainer
        const int minimumOffset = static_cast<int>(std::ceil(uiBase.getFontSize() * .5f));
        constrainer.setMinimumOnscreenAmounts(minimumOffset, minimumOffset, minimumOffset, minimumOffset);
    }

    void Dragger2D::sliderValueChanged(juce::Slider *slider) {
        juce::ignoreUnused(slider);
        const auto horizonP = horizonS.getNormalisableRange().convertTo0to1(horizonS.getValue());
        const auto verticalP = 1 - verticalS.getNormalisableRange().convertTo0to1(verticalS.getValue());
        const auto bound = getLocalBounds().toFloat();
        auto buttonBound = juce::Rectangle<float>(uiBase.getFontSize(), uiBase.getFontSize());
        buttonBound.setCentre(bound.getX() + static_cast<float>(horizonP) * bound.getWidth(),
                              bound.getY() + static_cast<float>(verticalP) * bound.getHeight());
        button.setBounds(buttonBound.toNearestInt());
    }
} // zlInterface
