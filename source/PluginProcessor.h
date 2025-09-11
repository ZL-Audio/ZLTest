#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
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
    enum OversampleIDx {
        kNone,
        kJUCE2,
        kJUCE8,
        kSmall2,
        kSmall8,
        kLarge2,
        kLarge8
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
    std::atomic<float> &flag;
    OversampleIDx c_flag;
    std::atomic<int> pdc_{0};

    std::array<float *, 2> pointers_{};

    juce::dsp::Oversampling<float> juce_oversampler2_{
        2, 1, juce::dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple, true, true
    };

    juce::dsp::Oversampling<float> juce_oversampler8_{
        2, 3, juce::dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple, true, true
    };

    zldsp::oversample::OverSampler<float, 1> oversampler2_small_{
        {zldsp::oversample::halfband_coeff::CoeffID::k96_07}
    };

    zldsp::oversample::OverSampler<float, 3> oversampler8_small_{
        {
            zldsp::oversample::halfband_coeff::CoeffID::k96_07,
            zldsp::oversample::halfband_coeff::CoeffID::k32_22,
            zldsp::oversample::halfband_coeff::CoeffID::k32_22
        }
    };

    zldsp::oversample::OverSampler<float, 1> oversampler2_{};
    zldsp::oversample::OverSampler<float, 3> oversampler8_{};

    void updateLatency();

    void handleAsyncUpdate() override;

    static void processSamples(float *samples, const size_t num_samples);
};
