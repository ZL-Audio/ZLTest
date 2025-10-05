#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      ), parameters(*this, nullptr, juce::Identifier("ZLTestParaState"),
                    {
                        std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("flag", 1),
                                                                     "Over Sample",
                                                                     juce::StringArray{
                                                                         "None",
                                                                         "JUCE 2*",
                                                                         "JUCE 8*",
                                                                         "Small 2*",
                                                                         "Small 8*",
                                                                         "Large 2*",
                                                                         "Large 8*"
                                                                     },
                                                                     0)
                    }),
      flag(*parameters.getRawParameterValue("flag")) {
}

PluginProcessor::~PluginProcessor() {
}

//==============================================================================
const juce::String PluginProcessor::getName() const {
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int PluginProcessor::getNumPrograms() {
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram() {
    return 0;
}

void PluginProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String PluginProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void PluginProcessor::changeProgramName(int index, const juce::String &newName) {
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::ignoreUnused(sampleRate);

    juce_oversampler2_.initProcessing(static_cast<size_t>(samplesPerBlock));
    juce_oversampler8_.initProcessing(static_cast<size_t>(samplesPerBlock));

    oversampler2_small_.prepare(2, static_cast<size_t>(samplesPerBlock));
    oversampler8_small_.prepare(2, static_cast<size_t>(samplesPerBlock));

    oversampler2_.prepare(2, static_cast<size_t>(samplesPerBlock));
    oversampler8_.prepare(2, static_cast<size_t>(samplesPerBlock));

    c_flag = static_cast<OversampleIDx>(std::round(flag.load(std::memory_order::relaxed)));
    updateLatency();
}

void PluginProcessor::updateLatency() {
    switch (c_flag) {
        case kNone: {
            pdc_.store(0);
            break;
        }
        case kJUCE2: {
            juce_oversampler2_.reset();
            pdc_.store(static_cast<int>(std::round(juce_oversampler2_.getLatencyInSamples())));
            break;
        }
        case kJUCE8: {
            juce_oversampler8_.reset();
            pdc_.store(static_cast<int>(std::round(juce_oversampler8_.getLatencyInSamples())));
            break;
        }
        case kSmall2: {
            oversampler2_small_.reset();
            pdc_.store(static_cast<int>(oversampler2_small_.getLatency()));
            break;
        }
        case kSmall8: {
            oversampler8_small_.reset();
            pdc_.store(static_cast<int>(oversampler8_small_.getLatency()));
            break;
        }
        case kLarge2: {
            oversampler2_.reset();
            pdc_.store(static_cast<int>(oversampler2_.getLatency()));
            break;
        }
        case kLarge8: {
            oversampler8_.reset();
            pdc_.store(static_cast<int>(oversampler8_.getLatency()));
            break;
        }
    }
    triggerAsyncUpdate();
}

void PluginProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void PluginProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                   juce::MidiBuffer &midiMessages) {
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    const auto new_flag = static_cast<OversampleIDx>(std::round(flag.load(std::memory_order::relaxed)));
    if (new_flag != c_flag) {
        c_flag = new_flag;
        updateLatency();
    }

    switch (c_flag) {
        case kNone: {
            for (size_t channel = 0; channel < 2; ++channel) {
                const auto pointer = buffer.getWritePointer(static_cast<int>(channel));
                processSamples(pointer, static_cast<size_t>(buffer.getNumSamples()));
            }
            break;
        }
        case kJUCE2: {
            juce::dsp::AudioBlock<float> block(buffer);
            const auto os_block = juce_oversampler2_.processSamplesUp(block);
            for (size_t channel = 0; channel < 2; ++channel) {
                const auto pointer = os_block.getChannelPointer(channel);
                processSamples(pointer, os_block.getNumSamples());
            }
            juce_oversampler2_.processSamplesDown(block);
            break;
        }
        case kJUCE8: {
            juce::dsp::AudioBlock<float> block(buffer);
            const auto os_block = juce_oversampler8_.processSamplesUp(block);
            for (size_t channel = 0; channel < 2; ++channel) {
                const auto pointer = os_block.getChannelPointer(channel);
                processSamples(pointer, os_block.getNumSamples());
            }
            juce_oversampler8_.processSamplesDown(block);
            break;
        }
        case kSmall2: {
            pointers_[0] = buffer.getWritePointer(0);
            pointers_[1] = buffer.getWritePointer(1);
            oversampler2_small_.upsample(pointers_, static_cast<size_t>(buffer.getNumSamples()));
            const auto &os_pointers = oversampler2_small_.getOSPointer();
            for (size_t channel = 0; channel < 2; ++channel) {
                processSamples(os_pointers[channel], static_cast<size_t>(buffer.getNumSamples()) << 1);
            }
            oversampler2_small_.downsample(pointers_, static_cast<size_t>(buffer.getNumSamples()));
            break;
        }
        case kSmall8: {
            pointers_[0] = buffer.getWritePointer(0);
            pointers_[1] = buffer.getWritePointer(1);
            oversampler8_small_.upsample(pointers_, static_cast<size_t>(buffer.getNumSamples()));
            const auto &os_pointers = oversampler8_small_.getOSPointer();
            for (size_t channel = 0; channel < 2; ++channel) {
                processSamples(os_pointers[channel], static_cast<size_t>(buffer.getNumSamples()) << 3);
            }
            oversampler8_small_.downsample(pointers_, static_cast<size_t>(buffer.getNumSamples()));
            break;
        }
        case kLarge2: {
            pointers_[0] = buffer.getWritePointer(0);
            pointers_[1] = buffer.getWritePointer(1);
            oversampler2_.upsample(pointers_, static_cast<size_t>(buffer.getNumSamples()));
            const auto &os_pointers = oversampler2_.getOSPointer();
            for (size_t channel = 0; channel < 2; ++channel) {
                processSamples(os_pointers[channel], static_cast<size_t>(buffer.getNumSamples()) << 1);
            }
            oversampler2_.downsample(pointers_, static_cast<size_t>(buffer.getNumSamples()));
            break;
        }
        case kLarge8: {
            pointers_[0] = buffer.getWritePointer(0);
            pointers_[1] = buffer.getWritePointer(1);
            oversampler8_.upsample(pointers_, static_cast<size_t>(buffer.getNumSamples()));
            const auto &os_pointers = oversampler8_.getOSPointer();
            for (size_t channel = 0; channel < 2; ++channel) {
                processSamples(os_pointers[channel], static_cast<size_t>(buffer.getNumSamples()) << 3);
            }
            oversampler8_.downsample(pointers_, static_cast<size_t>(buffer.getNumSamples()));
            break;
        }
    }
}

void PluginProcessor::processSamples(float *samples, const size_t num_samples) {
    juce::ignoreUnused(samples, num_samples);
    // for (size_t i = 0; i < num_samples; ++i) {
    //     samples[i] = std::tanh(samples[i]);
    // }
}

//==============================================================================
bool PluginProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *PluginProcessor::createEditor() {
    return new juce::GenericAudioProcessorEditor(*this);
    // return new PluginEditor(*this);
}

//==============================================================================
void PluginProcessor::getStateInformation(juce::MemoryBlock &destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused(destData);
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused(data, sizeInBytes);
}

void PluginProcessor::handleAsyncUpdate() {
    setLatencySamples(pdc_.load(std::memory_order::relaxed));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE

createPluginFilter() {
    return new PluginProcessor();
}
