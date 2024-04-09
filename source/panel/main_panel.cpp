//
// Created by Zishu Liu on 12/29/23.
//

#include "main_panel.h"

namespace zlPanel {
    MainPanel::MainPanel(PluginProcessor &p)
        : uiBase(), selector(uiBase, *this) {
        juce::ignoreUnused(p);
        viewPort.setScrollBarsShown(true, false,
                                    true, false);
        viewPort.setViewedComponent(&child, false);
        addAndMakeVisible(viewPort);
    }

    MainPanel::~MainPanel() = default;

    void MainPanel::paint(juce::Graphics &g) {
        juce::ignoreUnused(g);
        g.fillAll(juce::Colours::white);
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        // auto fontSize = bound.getHeight() * 0.0514f * 0.45f * 5;
        auto fontSize = bound.getHeight() * 0.0514f * 0.45f * 2.5f;
        uiBase.setFontSize(fontSize);
        // grid.items.add(colourSelector);
        // bound = bound.withSizeKeepingCentre(bound.getWidth() - 20 * fontSize,
        //                                     bound.getHeight());
        // bound.removeFromBottom(bound.getHeight() - 5 * fontSize);

        viewPort.setBounds(bound.toNearestInt());

        bound.setHeight(bound.getHeight() * 2.f);
        child.setBounds(bound.toNearestInt());

        // selector.setBounds(bound.toNearestInt());
    }
}
