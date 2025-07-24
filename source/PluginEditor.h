#pragma once

#include "PluginProcessor.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::Thread {
public:
    explicit PluginEditor(PluginProcessor &);

    ~PluginEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;

    void resized() override;

private:
    juce::VBlankAttachment vblank_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
    double start_stamp{-1.0}, time_stamp_{-1.0};

    void repaintCallBack(double time_stamp);

    void run() override;
};
