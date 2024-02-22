//
// Created by Zishu Liu on 2/17/24.
//

#ifndef ZLTest_LEFT_PANEL_HPP
#define ZLTest_LEFT_PANEL_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include "../PluginProcessor.h"
#include "../gui/gui.hpp"

namespace zlPanel {
    class LeftSubPanel final : public juce::Component,
                               public juce::ComponentListener {
    public:
        explicit LeftSubPanel(zlInterface::UIBase &base, bool isBuffered);

        void resized() override;

        void componentMovedOrResized(Component &component, bool wasMoved, bool wasResized) override;

    private:
        zlInterface::UIBase &uiBase;
        zlInterface::CompactLinearSlider slider1, slider2;

        constexpr static float width = 6.f, height = 4.f;
    };

    class LeftPanel final : public juce::Component {
    public:
        explicit LeftPanel(PluginProcessor &p, zlInterface::UIBase &base, bool isBuffered);

        void resized() override;

    private:
        zlInterface::UIBase &uiBase;
        zlInterface::Dragger dragger;

        LeftSubPanel subPanel;
    };
} // zlPanel

#endif //ZLTest_LEFT_PANEL_HPP
