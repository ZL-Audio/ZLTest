//
// Created by Zishu Liu on 12/29/23.
//

#include "main_panel.h"

namespace zlPanel {
    MainPanel::MainPanel(PluginProcessor &p) : uiBase() {
        juce::ignoreUnused(p);
        // slider2 = std::make_unique<zlInterface::TwoValueRotarySlider>("Freq", uiBase);
        // addAndMakeVisible(*slider2);
        // slider3 = std::make_unique<zlInterface::CompactLinearSlider>("Freq", uiBase);
        // slider3->getSlider().setDoubleClickReturnValue(false, 0.0, juce::ModifierKeys::noModifiers);
        // slider3->getSlider().setSliderSnapsToMousePosition(false);
        // addAndMakeVisible(*slider3);
        // button1 = std::make_unique<zlInterface::CompactButton>("B", uiBase);
        // addAndMakeVisible(*button1);
        // box1 = std::make_unique<zlInterface::RegularCombobox>("Slope", choices, uiBase);
        // addAndMakeVisible(*box1);
        // box2 = std::make_unique<zlInterface::CompactCombobox>("Slope", choices, uiBase);
        // addAndMakeVisible(*box2);
        dragger = std::make_unique<zlInterface::Dragger>(uiBase);
        auto *para1 = p.parameters.getParameter("gain1"), *para2 = p.parameters.getParameter("gain2");
        draggerAttachments.add(std::make_unique<zlInterface::DraggerParameterAttach>(
            *para1, para1->getNormalisableRange(),
            *para2, para2->getNormalisableRange(),
            *dragger, p.parameters.undoManager));
        // sliderAttachments.add(
        //     std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        //         p.parameters, "gain1", dragger->getHorizonS()));
        // sliderAttachments.add(
        //     std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        //         p.parameters, "gain2", dragger->getVerticalS()));
        addAndMakeVisible(*dragger);
    }

    MainPanel::~MainPanel() {
    }


    void MainPanel::paint(juce::Graphics &g) {
        juce::ignoreUnused(g);
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        auto fontSize = bound.getHeight() * 0.0514f * 0.45f * 5;
        uiBase.setFontSize(fontSize);
        dragger->setPadding(uiBase.getFontSize(), uiBase.getFontSize(),
                            uiBase.getFontSize(), uiBase.getFontSize());

        // slider2->setBounds(getLocalBounds());
        // slider3->getSlider().setMouseDragSensitivity(getLocalBounds().getWidth());
        // slider3->setBounds(getLocalBounds());
        // button1->setBounds(getLocalBounds());
        // bound.removeFromBottom(bound.getHeight() * 0.5f);
        // box1->setBounds(bound.toNearestInt());
        // box2->setBounds(bound.toNearestInt());
        dragger->setBounds(bound.toNearestInt());
    }
}
