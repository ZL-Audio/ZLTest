//
// Created by Zishu Liu on 12/29/23.
//

#include "main_panel.h"

namespace zlPanel {
    MainPanel::MainPanel(PluginProcessor &p) : uiBase() {
        slider2 = std::make_unique<zlInterface::TwoValueRotarySliderComponent>("Freq", uiBase);
        addAndMakeVisible(*slider2);
        // button1 = std::make_unique<zlInterface::CompactButton>("B", uiBase);
        // addAndMakeVisible(*button1);
    }

    MainPanel::~MainPanel(){
    }


    void MainPanel::paint(juce::Graphics &g) {
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        auto fontSize = bound.getHeight() * 0.0514f * 0.45f * 5;
        uiBase.setFontSize(fontSize);

        slider2->setBounds(getLocalBounds());
        // button1->setBounds(getLocalBounds());
    }

}
