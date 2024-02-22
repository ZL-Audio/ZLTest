//
// Created by Zishu Liu on 2/17/24.
//

#include "right_panel.hpp"

namespace zlPanel {
    RightSubPanel::RightSubPanel(zlInterface::UIBase &base, bool isBuffered)
        : uiBase(base),
          slider1("test1", base),
          slider2("test2", base) {
        addAndMakeVisible(slider1);
        addAndMakeVisible(slider2);
        setBufferedToImage(isBuffered);
    }

    void RightSubPanel::resized() {
        juce::Grid grid;
        using Track = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;

        grid.templateRows = {Track(Fr(1)), Track(Fr(1))};
        grid.templateColumns = {
            Track(Fr(30))
        };

        grid.items.add(slider1);
        grid.items.add(slider2);

        grid.setGap(juce::Grid::Px(uiBase.getFontSize()));
        grid.performLayout(getLocalBounds());
    }

    RightPanel::RightPanel(PluginProcessor &p, zlInterface::UIBase &base, bool isBuffered)
        : uiBase(base), callOutBoxLAF(base),
          isBufferedToImage(isBuffered) {
        juce::ignoreUnused(p);
        setBufferedToImage(isBuffered);
    }

    void RightPanel::paint(juce::Graphics &g) {
        g.fillAll(juce::Colours::black);
    }


    void RightPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        if (getParentComponent() == nullptr) {
            return;
        }
        auto content = std::make_unique<RightSubPanel>(uiBase, isBufferedToImage);
        content->setSize(juce::roundToInt(uiBase.getFontSize() * 6.f),
                         juce::roundToInt(uiBase.getFontSize() * 6.f));

        auto &box = juce::CallOutBox::launchAsynchronously(std::move(content),
                                                           getBounds(),
                                                           this);
        box.setLookAndFeel(&callOutBoxLAF);
        box.setArrowSize(0);
        box.sendLookAndFeelChange();
    }
} // zlPanel
