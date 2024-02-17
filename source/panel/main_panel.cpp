//
// Created by Zishu Liu on 12/29/23.
//

#include "main_panel.h"

namespace zlPanel {
    MainPanel::MainPanel(PluginProcessor &p) : uiBase() {
        juce::ignoreUnused(p);
        slider2 = std::make_unique<zlInterface::TwoValueRotarySlider>("Freq", uiBase);
        slider2->setShowSlider2(true);
        addAndMakeVisible(*slider2);
        // slider3 = std::make_unique<zlInterface::CompactLinearSlider>("Freq", uiBase);
        // slider3->getSlider().setDoubleClickReturnValue(false, 0.0, juce::ModifierKeys::noModifiers);
        // slider3->getSlider().setSliderSnapsToMousePosition(false);
        // addAndMakeVisible(*slider3);
        button1 = std::make_unique<zlInterface::CompactButton>("B", uiBase);
        addAndMakeVisible(*button1);
        // box1 = std::make_unique<zlInterface::RegularCombobox>("Slope", choices, uiBase);
        // addAndMakeVisible(*box1);
        box2 = std::make_unique<zlInterface::CompactCombobox>("Slope", choices, uiBase);
        addAndMakeVisible(*box2);
        dragger = std::make_unique<zlInterface::Dragger>(uiBase);
        auto *para1 = p.parameters.getParameter("gain1"), *para2 = p.parameters.getParameter("gain2");
        draggerAttachments.add(std::make_unique<zlInterface::DraggerParameterAttach>(
            *para1, para1->getNormalisableRange(),
            *para2, para2->getNormalisableRange(),
            *dragger, p.parameters.undoManager));
        addAndMakeVisible(*dragger);
    }

    MainPanel::~MainPanel() {
    }


    void MainPanel::paint(juce::Graphics &g) {
        juce::ignoreUnused(g);
        g.fillAll(juce::Colours::white);

        g.setColour(juce::Colours::black);
        g.setFont(uiBase.getFontSize() * 2);

        auto leftBound = getLocalBounds().toFloat();
        const auto rightBound = leftBound.removeFromRight(leftBound.getWidth() * .5f);

        g.drawText("test", leftBound, juce::Justification::centred);
        g.drawText("test", rightBound.toNearestInt(), juce::Justification::centred);
        // auto bound = getLocalBounds().toFloat();
        // const auto leftBound = bound.removeFromLeft(bound.getWidth() * .33333f);
        // const auto rightBound = bound.removeFromRight(bound.getWidth() * .5f);
        // uiBase.drawShadowEllipse(g, leftBound, uiBase.getFontSize(), {});
        // uiBase.drawInnerShadowEllipse(g, rightBound, uiBase.getFontSize(), {});
        // bound = uiBase.drawShadowEllipse(g, bound, uiBase.getFontSize(), {});
        // uiBase.drawInnerShadowEllipse(g, bound, uiBase.getFontSize() * .5f, {.flip = true});
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        auto fontSize = bound.getHeight() * 0.0514f * 0.45f * 5;
        uiBase.setFontSize(fontSize);
        // dragger->setPadding(uiBase.getFontSize(), uiBase.getFontSize(),
        //                     uiBase.getFontSize(), uiBase.getFontSize());

        // slider2->setBounds(getLocalBounds());
        // slider3->getSlider().setMouseDragSensitivity(getLocalBounds().getWidth());
        // slider3->setBounds(getLocalBounds());
        // button1->setBounds(getLocalBounds());
        // bound.removeFromBottom(bound.getHeight() * 0.5f);
        // box1->setBounds(bound.toNearestInt());
        // box2->setBounds(bound.toNearestInt());
        // dragger->setBounds(bound.toNearestInt());

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
