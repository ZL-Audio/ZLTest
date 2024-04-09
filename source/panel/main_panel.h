//
// Created by Zishu Liu on 12/29/23.
//

#ifndef MAIN_PANEL_H
#define MAIN_PANEL_H

#include <juce_audio_processors/juce_audio_processors.h>
#include "../PluginProcessor.h"
#include "../gui/gui.hpp"

namespace zlPanel {
    class MainPanel : public juce::Component {
    public:
        explicit MainPanel(PluginProcessor &p);

        ~MainPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

    private:
        zlInterface::UIBase uiBase;
        juce::Viewport viewPort;
        juce::Component child;
        zlInterface::ColourSelector selector;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainPanel)
    };
}


#endif //MAIN_PANEL_H
