//
// Created by Zishu Liu on 12/29/23.
//

#ifndef MAIN_PANEL_H
#define MAIN_PANEL_H

#include <juce_audio_processors/juce_audio_processors.h>
#include "../PluginProcessor.h"
#include "../gui/slider/rotarty_slider/rotary_slider_component.h"
#include "../gui/slider/two_value_rotary_slider/two_value_rotary_slider_component.h"
#include "../gui/button/compact_button/compact_button.h"

namespace zlPanel {

        class MainPanel : public juce::Component {
        public:
            explicit MainPanel(PluginProcessor &p);

            ~MainPanel() override;

            void paint(juce::Graphics &g) override;

            void resized() override;

        private:
            zlInterface::UIBase uiBase;

            std::unique_ptr<zlInterface::RotarySliderComponent> slider;

            std::unique_ptr<zlInterface::TwoValueRotarySliderComponent> slider2;

            std::unique_ptr<zlInterface::CompactButton> button1;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainPanel)
        };

}


#endif //MAIN_PANEL_H
