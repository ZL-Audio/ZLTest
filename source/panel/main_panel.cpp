//
// Created by Zishu Liu on 12/29/23.
//

#include "main_panel.h"

namespace zlPanel {
    MainPanel::MainPanel(PluginProcessor &p)
        : preocesorRef(p), uiBase(),
          leftPanel1(p, uiBase, true), leftPanel2(p, uiBase, false),
          rightPanel1(p, uiBase, true), rightPanel2(p, uiBase, false),
          lPanel1(uiBase, true), lPanel2(uiBase, false),
    name_look_and_feel_(uiBase) {
        juce::ignoreUnused(p);
       label.setLookAndFeel(&name_look_and_feel_);
        // addAndMakeVisible(leftPanel1);
        // addAndMakeVisible(leftPanel2);
        // addAndMakeVisible(rightPanel1);
        // addAndMakeVisible(rightPanel2);
        // addAndMakeVisible(lPanel1);
        // addAndMakeVisible(lPanel2);
        name_look_and_feel_.setFontScale(3.f);
        addAndMakeVisible(label);
        startTimer(10);
    }

    MainPanel::~MainPanel() {
        label.setLookAndFeel(nullptr);
        stopTimer();
    }

    void MainPanel::paint(juce::Graphics &g) {
        juce::ignoreUnused(g);
        g.fillAll(uiBase.getBackgroundColor());
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
        // auto fontSize = bound.getHeight() * 0.0514f * 0.45f * 5;
        auto fontSize = bound.getHeight() * 0.0514f * 0.45f * 2.5f;
        uiBase.setFontSize(fontSize);
        label.setBounds(getLocalBounds());

        // juce::Grid grid;
        // using Track = juce::Grid::TrackInfo;
        // using Fr = juce::Grid::Fr;
        //
        // grid.templateRows = {Track(Fr(1)), Track(Fr(1))};
        // grid.templateColumns = {
        //     Track(Fr(30)), Track(Fr(30)), Track(Fr(30)),
        // };
        //
        // grid.items.add(leftPanel1);
        // grid.items.add(lPanel1);
        // grid.items.add(rightPanel1);
        // grid.items.add(leftPanel2);
        // grid.items.add(lPanel2);
        // grid.items.add(rightPanel2);
        //
        //
        // grid.performLayout(bound.toNearestInt());
    }

    void MainPanel::timerCallback() {
        label.setText(juce::String(preocesorRef.blockSize.load()), juce::sendNotification);
    }

}
