#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "fft_engine/fft_engine.hpp"

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor {
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)

    static constexpr size_t fft_order = 13;
    static constexpr size_t fft_size = static_cast<size_t>(1) << fft_order;

    size_t overlap = 4; // 75% overlap
    size_t hopSize = fft_size / overlap;
    static constexpr float windowCorrection = 2.0f / 3.0f;
    static constexpr float bypassCorrection = 1.0f / 4.0f;
    // counts up until the next hop.
    size_t count = 0;
    // write position in input FIFO and read position in output FIFO.
    size_t pos = 0;
    // circular buffers for incoming and outgoing audio data.
    std::vector<std::vector<float> > inputFIFOs, outputFIFOs;

    std::vector<float> in_buffer;
    std::vector<std::complex<float>> out_buffer;
    std::vector<std::complex<float>> dummy_spectrum;
    zlFFTEngine::KFREngine<float> kfr_engine;
    zlFFTEngine::JUCEEngine<float> juce_engine;
    std::unique_ptr<juce::dsp::WindowingFunction<float> > window;

    void processFrame();

    void processSpectrum();
};
