//
// Created by Zishu Liu on 12/29/23.
//

#include "main_panel.h"

namespace zlPanel {
    MainPanel::MainPanel(PluginProcessor &p) : uiBase() {
        juce::ignoreUnused(p);
        // slider2 = std::make_unique<zlInterface::TwoValueRotarySlider>("Freq", uiBase);
        // addAndMakeVisible(*slider2);
        slider3 = std::make_unique<zlInterface::CompactLinearSlider>("Freq", uiBase);
        addAndMakeVisible(*slider3);
        // button1 = std::make_unique<zlInterface::CompactButton>("B", uiBase);
        // addAndMakeVisible(*button1);
    }

    MainPanel::~MainPanel(){
    }


    void MainPanel::paint(juce::Graphics &g) {
        juce::ignoreUnused(g);
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        auto fontSize = bound.getHeight() * 0.0514f * 0.45f * 5;
        uiBase.setFontSize(fontSize);

        // slider2->setBounds(getLocalBounds());
        slider3->setBounds(getLocalBounds());
        // button1->setBounds(getLocalBounds());
    }

}
