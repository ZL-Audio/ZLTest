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
                        std::make_unique<juce::AudioParameterBool>("flag", // parameterID
                                                                   "Over Sample", // parameter name
                                                                   false) // default value
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
    stage_.prepare(2, static_cast<size_t>(samplesPerBlock));
    oversampler_.initProcessing(static_cast<size_t>(samplesPerBlock));

    if (old_flag) {
        pdc_.store(static_cast<int>(stage_.getLatency()));
    } else {
        pdc_.store(static_cast<int>(std::round(oversampler_.getLatencyInSamples())));
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
    const auto new_flag = flag.load(std::memory_order::relaxed) > .5f;
    if (new_flag != old_flag) {
        old_flag = new_flag;
        if (old_flag) {
            pdc_.store(static_cast<int>(stage_.getLatency()));
        } else {
            pdc_.store(static_cast<int>(std::round(oversampler_.getLatencyInSamples())));
        }
        triggerAsyncUpdate();
    }

    if (old_flag) {
        std::array<float *, 2> pointers = {buffer.getWritePointer(0), buffer.getWritePointer(1)};
        stage_.upsample(pointers, static_cast<size_t>(buffer.getNumSamples()));
        auto &os_buffer = stage_.getOSBuffer();
        // for (size_t channel = 0; channel < 2; ++channel) {
        //     auto vector = kfr::make_univector(os_buffer[channel]);
        //     vector = kfr::sin(1.5707963267948965f * vector);
        // }
        stage_.downsample(pointers, static_cast<size_t>(buffer.getNumSamples()));
    } else {
        juce::dsp::AudioBlock<float> block(buffer);
        oversampler_.processSamplesUp(block);
        oversampler_.processSamplesDown(block);
    }
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
