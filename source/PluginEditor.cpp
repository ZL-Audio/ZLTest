#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p)
    : AudioProcessorEditor(&p), Thread("zltest"),
      vblank_(this, [this](const double x) { repaintCallBack(x); }) {
    setSize(300, 185);
    getConstrainer()->setFixedAspectRatio(1.f);
    setResizable(true, p.wrapperType != PluginProcessor::wrapperType_AudioUnitv3);

    startThread(juce::Thread::Priority::low);
}

PluginEditor::~PluginEditor() {
    if (isThreadRunning()) {
        stopThread(-1);
    }
}

void PluginEditor::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    auto bound = getLocalBounds().toFloat();
    const auto height = bound.getHeight();
    g.setFont(height * .375f);
    const auto seconds = static_cast<size_t>(std::max(0.0, std::floor(time_stamp_ - start_stamp)));
    auto hour_string = std::to_string(seconds / 60);
    while (hour_string.length() < 2) {
        hour_string.insert(0, "0");
    }
    auto second_string = std::to_string(seconds % 60);
    while (second_string.length() < 2) {
        second_string.insert(0, "0");
    }
    g.drawText(hour_string + ":" + second_string,
               getLocalBounds(), juce::Justification::centred);
}

void PluginEditor::resized() {
}

void PluginEditor::repaintCallBack(const double time_stamp) {
    if (start_stamp < 0.0) {
        start_stamp = time_stamp;
        return;
    }
    if (time_stamp - start_stamp >= std::ceil(time_stamp_ - start_stamp)) {
        time_stamp_ = time_stamp;
        repaint();
    }
}

void PluginEditor::run() {
    while (true) {
        juce::Thread::sleep(10);
        if (threadShouldExit()) {
            break;
        }
    }
}
