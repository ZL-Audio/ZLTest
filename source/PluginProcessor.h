#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "over_sample/over_sample.hpp"

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor,
                        private juce::AsyncUpdater {
public:
    juce::AudioProcessorValueTreeState parameters;

    PluginProcessor();

    ~PluginProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    juce::AudioProcessorEditor *createEditor() override;

    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;

    bool producesMidi() const override;

    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    int getNumPrograms() override;

    int getCurrentProgram() override;

    void setCurrentProgram(int index) override;

    const juce::String getProgramName(int index) override;

    void changeProgramName(int index, const juce::String &newName) override;

    void getStateInformation(juce::MemoryBlock &destData) override;

    void setStateInformation(const void *data, int sizeInBytes) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
    std::atomic<float> &flag;
    bool old_flag{false};
    std::atomic<int> pdc_{0};
    kfr::univector<float, 53> taps = {
        0.0, 0.00011071833641849496, 0.0, -0.00034292125098919817, 0.0, 0.0008340730541547668, 0.0,
        -0.0017335249370894709, 0.0, 0.003244362402711567, 0.0, -0.005630398866738123, 0.0, 0.009242395565167542, 0.0,
        -0.014586502646620205, 0.0, 0.02250479985326706, 0.0, -0.034695176940198295, 0.0, 0.05551339943988561, 0.0,
        -0.10101812841049512, 0.0, 0.31655390623272733, 0.5000711190817596, 0.31655390623272733, 0.0,
        -0.10101812841049512, 0.0, 0.05551339943988561, 0.0, -0.034695176940198295, 0.0, 0.02250479985326706, 0.0,
        -0.014586502646620205, 0.0, 0.009242395565167542, 0.0, -0.005630398866738123, 0.0, 0.003244362402711567, 0.0,
        -0.0017335249370894709, 0.0, 0.0008340730541547668, 0.0, -0.00034292125098919817, 0.0, 0.00011071833641849496,
        0.0
    };
    kfr::fir_filter<float> f1{taps}, f2{taps};

    zldsp::oversample::OverSampleStage<float> stage_{
        std::span(zldsp::oversample::kCoeff_64_08_80_float),
        std::span(zldsp::oversample::kCoeff_64_08_80_float)
    };

    void handleAsyncUpdate() override;
};
