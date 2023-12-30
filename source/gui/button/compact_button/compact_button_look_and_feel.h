//
// Created by Zishu Liu on 12/29/23.
//

#ifndef COMPACT_BUTTON_LOOK_AND_FEEL_H
#define COMPACT_BUTTON_LOOK_AND_FEEL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.h"

namespace zlInterface {
    class CompactButtonLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        explicit CompactButtonLookAndFeel(UIBase &base) : uiBase(base) {
        }

        void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button, bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override {
            juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
            g.fillAll(uiBase.getBackgroundColor());

            auto bounds = button.getLocalBounds().toFloat();
            bounds = uiBase.drawShadowEllipse(g, bounds, uiBase.getFontSize() * 0.5f, {});
            bounds = uiBase.drawInnerShadowEllipse(g, bounds, uiBase.getFontSize() * 0.15f, {.flip = true});
            if (button.getToggleState()) {
                bounds = uiBase.getShadowEllipseArea(bounds, uiBase.getFontSize() * 0.2f, {});
                bounds = uiBase.drawShadowEllipse(g, bounds, uiBase.getFontSize() * 0.05f,
                                                  {.flip = true, .fit = false});
                uiBase.drawInnerShadowEllipse(g, bounds, uiBase.getFontSize() * 0.5f, {});
            }
            if (editable.load()) {
                auto textBound = button.getLocalBounds().toFloat();
                if (button.getToggleState()) {
                    textBound.removeFromTop(uiBase.getFontSize() * 0.0625f);
                    shouldDrawButtonAsDown ? g.setColour(uiBase.getTextInactiveColor()) : g.setColour(uiBase.getTextColor());
                } else {
                    textBound.removeFromBottom(uiBase.getFontSize() * 0.0625f);
                    shouldDrawButtonAsDown ? g.setColour(uiBase.getTextColor()) : g.setColour(uiBase.getTextInactiveColor());
                }
                g.setFont(uiBase.getFontSize() * FontLarge);
                g.drawText(button.getButtonText(), textBound.toNearestInt(), juce::Justification::centred);
            }
        }

        inline void setEditable(bool f) {
            editable.store(f);
        }

    private:
        std::atomic<bool> editable = true;

        UIBase &uiBase;
    };
}

#endif //COMPACT_BUTTON_LOOK_AND_FEEL_H
