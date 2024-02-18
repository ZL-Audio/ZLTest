//
// Created by Zishu Liu on 12/29/23.
//

#include "main_panel.h"

namespace zlPanel {
    MainPanel::MainPanel(PluginProcessor &p)
        : uiBase(),
          leftPanel(p, uiBase),
          rightPanel(p, uiBase) {
        juce::ignoreUnused(p);
        addAndMakeVisible(leftPanel);
        addAndMakeVisible(rightPanel);
    }

    MainPanel::~MainPanel() {
    }

    void MainPanel::paint(juce::Graphics &g) {
        juce::ignoreUnused(g);
        g.fillAll(juce::Colours::white);
        // auto bounds = getLocalBounds().toFloat();
        // bounds = uiBase.drawShadowEllipse(g, bounds, uiBase.getFontSize() * 1.f, {});
        // bounds = uiBase.drawInnerShadowEllipse(g, bounds, uiBase.getFontSize() * 0.15f, {.flip = true});
        // g.setFont(uiBase.getFontSize());
        // g.setColour(juce::Colours::black);
        // g.drawText("test", bounds, juce::Justification::centredTop);
        // g.drawText("test", bounds.toNearestInt(), juce::Justification::centredBottom);
        // g.setColour(juce::Colours::black.withAlpha(1.f));
        // g.drawText("test", bounds.toNearestInt(), juce::Justification::centred);
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        auto fontSize = bound.getHeight() * 0.0514f * 0.45f * 5;
        uiBase.setFontSize(fontSize);

        leftPanel.setBounds(bound.removeFromLeft(bound.getWidth() * .5f).toNearestInt());

        bound.removeFromBottom(bound.getHeight() * .8f);

        rightPanel.setBounds(bound.toNearestInt());
        //
        // juce::Grid grid;
        // using Track = juce::Grid::TrackInfo;
        // using Fr = juce::Grid::Fr;
        //
        // grid.templateRows = {Track(Fr(1)), Track(Fr(1))};
        // grid.templateColumns = {
        //     Track(Fr(30)), Track(Fr(30)), Track(Fr(30)), Track(Fr(30)),
        // };
        //
        // grid.items = {
        //     juce::GridItem(*slider2).withArea(1, 1, 3, 3),
        //     juce::GridItem(*button1).withArea(1, 3),
        //     juce::GridItem(*box2).withArea(1, 4),
        //     juce::GridItem(*dragger).withArea(2, 3, 3, 5)
        // };
        //
        // grid.setGap(juce::Grid::Px(uiBase.getFontSize()));
        // grid.performLayout(bound.toNearestInt());
    }
}
