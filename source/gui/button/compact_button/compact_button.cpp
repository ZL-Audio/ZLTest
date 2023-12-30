//
// Created by Zishu Liu on 12/29/23.
//

#include "compact_button.h"

namespace zlInterface {
    CompactButton::CompactButton(const juce::String &labelText, UIBase &base) : uiBase(base), lookAndFeel(uiBase) {
        button.setClickingTogglesState(true);
        button.setButtonText(labelText);
        button.setLookAndFeel(&lookAndFeel);
        addAndMakeVisible(button);
    }

    CompactButton::~CompactButton() {
        button.setLookAndFeel(nullptr);
    }

    void CompactButton::resized() {
        button.setBounds(getLocalBounds());
    }

}
