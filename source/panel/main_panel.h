//
// Created by Zishu Liu on 12/29/23.
//

#ifndef MAIN_PANEL_H
#define MAIN_PANEL_H

#include <juce_audio_processors/juce_audio_processors.h>
#include "../PluginProcessor.h"
#include "../gui/slider/slider.h"
#include "../gui/button/button.h"
#include "../gui/combobox/combobox.h"

namespace zlPanel {

        class MainPanel : public juce::Component {
        public:
            explicit MainPanel(PluginProcessor &p);

            ~MainPanel() override;

            void paint(juce::Graphics &g) override;

            void resized() override;

        private:
            zlInterface::UIBase uiBase;

            std::unique_ptr<zlInterface::RotarySlider> slider;

            std::unique_ptr<zlInterface::TwoValueRotarySlider> slider2;

            std::unique_ptr<zlInterface::CompactLinearSlider> slider3;

            std::unique_ptr<zlInterface::CompactButton> button1;

            std::unique_ptr<zlInterface::RegularCombobox> box1;
            std::unique_ptr<zlInterface::CompactCombobox> box2;
            juce::StringArray choices = {"6", "12", "18"};

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainPanel)
        };

}


#endif //MAIN_PANEL_H
