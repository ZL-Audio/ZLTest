#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p)
    : AudioProcessorEditor(&p), Thread("zltest"),
      vblank_(this, [this](const double x) { repaintCallBack(x); }) {
    setSize(300, 185);
    // getConstrainer()->setFixedAspectRatio(1.f);
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

    auto bound = getLocalBounds().toFloat();
    juce::Path path;
    const auto temp_bound = bound.reduced(5.f);
    path.startNewSubPath(temp_bound.getBottomLeft());
    path.lineTo(temp_bound.getTopRight());
    path.lineTo(temp_bound.getBottomRight());

    g.setColour(juce::Colours::orange);
    g.fillPath(path);
    g.setColour(juce::Colours::white);
    g.strokePath(path, juce::PathStrokeType{2.5f});

    return;

    g.setColour(juce::Colours::white);
    g.setFont(std::min(bound.getHeight() * .75f, bound.getWidth() * .25f));
    const auto seconds = static_cast<size_t>(std::max(0.0, std::floor(time_stamp_ - start_stamp)));
    auto hour_string = std::to_string((seconds / 60) % 100);
    while (hour_string.length() < 2) {
        hour_string.insert(0, "0");
    }
    auto second_string = std::to_string(seconds % 60);
    while (second_string.length() < 2) {
        second_string.insert(0, "0");
    }
    auto left_bound = getLocalBounds().toFloat();
    const auto right_bound = left_bound.removeFromRight(left_bound.getWidth() * .5f);
    const auto center_bound = getLocalBounds().toFloat().withSizeKeepingCentre(
        left_bound.getWidth(), left_bound.getHeight());
    g.drawText(hour_string + " ", left_bound, juce::Justification::centredRight);
    g.drawText(":", center_bound, juce::Justification::centred);
    g.drawText(" " + second_string, right_bound, juce::Justification::centredLeft);
}

void PluginEditor::resized() {
}

void PluginEditor::mouseDown(const juce::MouseEvent&) {
    start_stamp = -1.0;
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
