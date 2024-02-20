//
// Created by Zishu Liu on 2/17/24.
//

#ifndef ZLTest_RIGHT_PANEL_HPP
#define ZLTest_RIGHT_PANEL_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include "../PluginProcessor.h"
#include "../gui/gui.hpp"

namespace zlPanel {
    class RightSubPanel final : public juce::Component,
                                public juce::ComponentListener {
    public:
        explicit RightSubPanel(zlInterface::UIBase &base);

        void resized() override;

    private:
        zlInterface::UIBase &uiBase;
        zlInterface::CompactLinearSlider slider1, slider2;
    };

    class RightPanel final : public juce::Component {
    public:
        RightPanel(PluginProcessor &p, zlInterface::UIBase &base);

        void paint(juce::Graphics &g) override;

        void mouseDown(const juce::MouseEvent &event) override;

    private:
        zlInterface::UIBase &uiBase;
        zlInterface::CallOutBoxLAF callOutBoxLAF;
    };
} // zlPanel

#endif //ZLTest_RIGHT_PANEL_HPP
