#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p)
        : AudioProcessorEditor(&p), processorRef(p) {
    juce::ignoreUnused(processorRef);

    setSize(300, 300);
    getConstrainer()->setFixedAspectRatio(1.f);
    setResizable(true, p.wrapperType != PluginProcessor::wrapperType_AudioUnitv3);
}

PluginEditor::~PluginEditor() {
}

void PluginEditor::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    if (engines.size() == 0) return;
    auto bound = getLocalBounds().toFloat();
    const auto height = bound.getHeight() / static_cast<float>(engines.size());
    for (int i = 0; i < engines.size(); ++i) {
        const auto engineBound = bound.removeFromTop(height);
        g.setFont(height * .125f);
        g.drawText(engines[i], engineBound, juce::Justification::centred);
    }
}

void PluginEditor::resized() {
}

void PluginEditor::parentHierarchyChanged() {
    if (const auto peer = getPeer()) {
        engines = peer->getAvailableRenderingEngines();
    }
    repaint();
}
