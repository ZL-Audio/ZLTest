//
// Created by Zishu Liu on 2/17/24.
//

#include "left_panel.hpp"

namespace zlPanel {
    LeftSubPanel::LeftSubPanel(zlInterface::UIBase &base)
        : uiBase(base),
          slider1("test1", base),
          slider2("test2", base) {
        addAndMakeVisible(slider1);
        addAndMakeVisible(slider2);
    }

    void LeftSubPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        const auto leftBound = bound.removeFromTop(bound.getHeight() * .5f);
        slider1.setBounds(leftBound.toNearestInt());
        slider2.setBounds(bound.toNearestInt());
    }

    void LeftSubPanel::componentMovedOrResized(Component &component, bool wasMoved, bool wasResized) {
        juce::ignoreUnused(wasMoved, wasResized);
        if (getParentComponent() == nullptr || component.getParentComponent() == nullptr) {
            return;
        }
        const auto compBound = component.getBoundsInParent().toFloat();
        const auto compParentBound = component.getParentComponent()->getLocalBounds().toFloat();
        const auto shiftX = compBound.getCentreX() - compParentBound.getCentreX();
        const auto shiftY = compBound.getCentreY() - compParentBound.getCentreY();

        const auto bound = getParentComponent()->getLocalBounds().toFloat();
        const auto finalY = bound.getCentreY() - height * uiBase.getFontSize() + shiftY;
        const auto finalX = juce::jlimit(bound.getX() + width * uiBase.getFontSize() / 2,
                                         bound.getRight() - width * uiBase.getFontSize() / 2,
                                         bound.getCentreX() + shiftX);

        const auto popUpBound = juce::Rectangle<float>(
            width * uiBase.getFontSize(),
            height * uiBase.getFontSize()).withCentre({finalX, finalY});
        setBounds(popUpBound.toNearestInt());
    }

    LeftPanel::LeftPanel(PluginProcessor &p, zlInterface::UIBase &base)
        : uiBase(base), dragger(base), subPanel(base) {
        juce::ignoreUnused(p);
        addAndMakeVisible(dragger);
        dragger.getButton().addComponentListener(&subPanel);
        addChildComponent(subPanel);
        subPanel.setAlwaysOnTop(true);
    }

    void LeftPanel::resized() {
        dragger.setBounds(getLocalBounds());
    }

} // zlPanel
