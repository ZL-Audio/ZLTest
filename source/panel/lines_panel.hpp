//
// Created by Zishu Liu on 2/22/24.
//

#ifndef ZLTest_LINES_PANEL_HPP
#define ZLTest_LINES_PANEL_HPP

#include "../gui/gui.hpp"

namespace zlPanel {
    class LinesPanel : public juce::Component {
    public:
        explicit LinesPanel(zlInterface::UIBase &base, bool isBuffered);

        void paint(juce::Graphics &g) override;

    private:
        zlInterface::UIBase &uiBase;
    };
} // zlPanel

#endif //ZLTest_LINES_PANEL_HPP
