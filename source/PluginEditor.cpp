#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p)
        : AudioProcessorEditor(&p), processorRef(p), mainPanel(p) {
    juce::ignoreUnused(processorRef);

    // set font
    const auto sourceCodePro = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MiSansLatinMedium_ttf, BinaryData::MiSansLatinMedium_ttfSize);
    juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(sourceCodePro);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 200);
    addAndMakeVisible(mainPanel);

    getConstrainer()->setFixedAspectRatio(2.f);
    setResizable(true, p.wrapperType != PluginProcessor::wrapperType_AudioUnitv3);
}

PluginEditor::~PluginEditor() {
}

void PluginEditor::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    juce::ignoreUnused(g);
}

void PluginEditor::resized() {
    // layout the positions of your child components here
    mainPanel.setBounds(getLocalBounds());
}
