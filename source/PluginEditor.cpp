#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p)
        : AudioProcessorEditor(&p), processorRef(p) {
    juce::ignoreUnused(processorRef);

    setSize(400, 150);
    getConstrainer()->setFixedAspectRatio(400.f / 150.f);
    setResizable(true, p.wrapperType != PluginProcessor::wrapperType_AudioUnitv3);
    startTimerHz(60);
}

PluginEditor::~PluginEditor() {
    stopTimer();
}

void PluginEditor::paint(juce::Graphics &g) {
    const auto time = juce::Time::getCurrentTime();
    const auto timeString = time.toString(false, true, true, true);
    g.setFont(getLocalBounds().toFloat().getHeight() * .5f);

    if (colourFlag == 0) {
        g.fillAll(juce::Colours::white);
        g.setColour(juce::Colours::black);
        g.drawText(timeString, getLocalBounds(), juce::Justification::centred);
    } else {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::white);
        g.drawText(timeString, getLocalBounds(), juce::Justification::centred);
    }
}

void PluginEditor::mouseDown(const juce::MouseEvent &event) {
    juce::ignoreUnused(event);
    colourFlag = 1 - colourFlag;
}

void PluginEditor::resized() {
}

void PluginEditor::timerCallback() {
    repaint();
}
