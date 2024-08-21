#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor,
juce::Timer {
public:
    explicit PluginEditor(PluginProcessor &);

    ~PluginEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;

    void resized() override;

    void mouseDown(const juce::MouseEvent &event) override;

private:
    PluginProcessor &processorRef;

    int colourFlag = 0;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
