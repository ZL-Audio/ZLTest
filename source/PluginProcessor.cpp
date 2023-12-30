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
),
          parameters(*this, nullptr, juce::Identifier("ZLTestParas"),
                     {
                             std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("filter_type", 1),
                                                                          "Filter Type",
                                                                          juce::StringArray{"FIR", "IIR"},
                                                                          0),
                             std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("output_band", 1),
                                                                          "Output Band",
                                                                          juce::StringArray{"Low", "Mid", "High",
                                                                                            "All"},
                                                                          0),
                             std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("low_split", 1),
                                                                         "Low Split",
                                                                         juce::NormalisableRange<float>(100, 10000, 1),
                                                                         240),
                             std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("high_split", 1),
                                                                         "High Split",
                                                                         juce::NormalisableRange<float>(100, 10000, 1),
                                                                         4800),
                     }),
          crossover(*this, 64), lrCrossover(*this) {
    parameters.addParameterListener("filter_type", this);
    parameters.addParameterListener("low_split", this);
    parameters.addParameterListener("high_split", this);
}

void PluginProcessor::parameterChanged(const juce::String &parameterID, float newValue) {
    if (parameterID == "low_split") {
        crossover.setLowFreq(newValue);
        lrCrossover.setLowFreq(newValue);
    } else if (parameterID == "high_split") {
        crossover.setHighFreq(newValue);
        lrCrossover.setLowFreq(newValue);
    } else if (parameterID == "filter_type") {
        filterType.store(static_cast<size_t>(newValue));
        if (filterType.load() == 0) {
            setLatencySamples(crossover.getLatency());
        } else {
            setLatencySamples(0);
        }
    }
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
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
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
    auto channels = static_cast<juce::uint32> (juce::jmin(getMainBusNumInputChannels(),
                                                          getMainBusNumOutputChannels()));
    juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32> (samplesPerBlock),
                                channels};
    crossover.prepare(spec);
    lrCrossover.prepare(spec);
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
    juce::ignoreUnused(buffer);

//    juce::dsp::AudioBlock<float> block(buffer);
//    auto outputBand = static_cast<size_t>(parameters.getRawParameterValue("output_band")->load());
//    if (filterType.load() == 0) {
//        crossover.split(block);
//        if (outputBand == 0) {
//            block.copyFrom(crossover.getLowBlock());
//        } else if (outputBand == 1) {
//            block.copyFrom(crossover.getMidBlock());
//        } else if (outputBand == 2) {
//            block.copyFrom(crossover.getHighBlock());
//        } else {
//            crossover.combine(block);
//        }
//    } else {
//        lrCrossover.split(block);
//        if (outputBand == 0) {
//            block.copyFrom(lrCrossover.getLowBlock());
//        } else if (outputBand == 1) {
//            block.copyFrom(lrCrossover.getMidBlock());
//        } else if (outputBand == 2) {
//            block.copyFrom(lrCrossover.getHighBlock());
//        } else {
//            lrCrossover.combine(block);
//        }
//    }
}

//==============================================================================
bool PluginProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *PluginProcessor::createEditor() {
    return new PluginEditor(*this);
    // return new juce::GenericAudioProcessorEditor(*this);
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

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE

createPluginFilter() {
    return new PluginProcessor();
}
