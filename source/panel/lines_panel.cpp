//
// Created by Zishu Liu on 2/22/24.
//

#include "lines_panel.hpp"

namespace zlPanel {

    LinesPanel::LinesPanel(zlInterface::UIBase &base, const bool isBuffered)
        : uiBase(base){
        juce::ignoreUnused(uiBase);
        setBufferedToImage(isBuffered);
    }

    void LinesPanel::paint(juce::Graphics &g) {
        auto bound = getLocalBounds().toFloat();
        auto thickness = uiBase.getFontSize() * .125f * .125f;
        const auto interval = bound.getHeight() * .1f;
        g.setColour(juce::Colours::black);
        for (auto i = 0; i < 9; ++i) {
            bound.removeFromTop(interval);
            const juce::Line<float> line{bound.getTopLeft(), bound.getTopRight()};
            g.drawLine(line, thickness);
            thickness *= 1.5f;
        }
    }
} // zlPanel