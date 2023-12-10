//
// Created by Zishu Liu on 12/9/23.
//

#include "fir_crossover.h"

namespace zl_crossover {
    template<typename FloatType>
    FIRCrossover<FloatType>::FIRCrossover(juce::AudioProcessor &processorRef, size_t filterOrder) {
        p = &processorRef;
        order = filterOrder;
    }

    template<typename FloatType>
    void FIRCrossover<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        filterSpec = spec;
        lowGain.prepare(spec);
        lowMidGain.prepare(spec);
        allPass.prepare(spec);
        allPass.setMaximumDelayInSamples(getLatency());
        allPass.setDelay(static_cast<float>(getLatency()));
        lowPass.prepare(spec);
        setLowFreq(lowFreq);
        lowMidPass.prepare(spec);
        setHighFreq(highFreq);

        lowBuffer.setSize(static_cast<int>(spec.numChannels),
                          static_cast<int>(spec.maximumBlockSize));
        midBuffer.setSize(static_cast<int>(spec.numChannels),
                          static_cast<int>(spec.maximumBlockSize));
        highBuffer.setSize(static_cast<int>(spec.numChannels),
                           static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    void FIRCrossover<FloatType>::setLowFreq(FloatType freq) {
        lowFreq = freq;
//        auto coeff = juce::dsp::FilterDesign<FloatType>::designFIRLowpassWindowMethod(freq,
//                                                                                      filterSpec.sampleRate,
//                                                                                      order,
//                                                                                      juce::dsp::WindowingFunction<FloatType>::blackmanHarris);
        auto coeff = juce::dsp::FilterDesign<FloatType>::designFIRLowpassTransitionMethod(freq,
                                                                                          filterSpec.sampleRate,
                                                                                          order,
                                                                                          FloatType(0.1),
                                                                                          FloatType(3.0));
        const juce::GenericScopedLock<juce::CriticalSection> processLock(p->getCallbackLock());

        lowGain.setGainLinear(
                static_cast<float>(1 / coeff.get()->getMagnitudeForFrequency(0.1, filterSpec.sampleRate)));
        *lowPass.state = *coeff;
    }

    template<typename FloatType>
    void FIRCrossover<FloatType>::setHighFreq(FloatType freq) {
        highFreq = freq;
//        auto coeff = juce::dsp::FilterDesign<FloatType>::designFIRLowpassWindowMethod(freq,
//                                                                                      filterSpec.sampleRate,
//                                                                                      order,
//                                                                                      juce::dsp::WindowingFunction<FloatType>::blackmanHarris);
        auto coeff = juce::dsp::FilterDesign<FloatType>::designFIRLowpassTransitionMethod(freq,
                                                                                          filterSpec.sampleRate,
                                                                                          order,
                                                                                          FloatType(0.2),
                                                                                          FloatType(3.0));
        const juce::GenericScopedLock<juce::CriticalSection> processLock(p->getCallbackLock());

        lowMidGain.setGainLinear(
                static_cast<float>(1 / coeff.get()->getMagnitudeForFrequency(0.1, filterSpec.sampleRate)));
        *lowMidPass.state = *coeff;
    }

    template<typename FloatType>
    void FIRCrossover<FloatType>::split(juce::dsp::AudioBlock<FloatType> &block) {
        auto lowBlock = juce::dsp::AudioBlock<FloatType>(lowBuffer);
        auto midBlock = juce::dsp::AudioBlock<FloatType>(midBuffer);
        auto highBlock = juce::dsp::AudioBlock<FloatType>(highBuffer);
        lowBlock.copyFrom(block);
        midBlock.copyFrom(block);
        highBlock.copyFrom(block);

        lowPass.process(juce::dsp::ProcessContextReplacing<FloatType>(lowBlock));
        lowGain.process(juce::dsp::ProcessContextReplacing<FloatType>(lowBlock));

        lowMidPass.process(juce::dsp::ProcessContextReplacing<FloatType>(midBlock));
        lowMidGain.process(juce::dsp::ProcessContextReplacing<FloatType>(midBlock));

        allPass.process(juce::dsp::ProcessContextReplacing<FloatType>(highBlock));

        highBlock.replaceWithDifferenceOf(highBlock, midBlock);
        midBlock.replaceWithDifferenceOf(midBlock, lowBlock);
    }

    template<typename FloatType>
    void FIRCrossover<FloatType>::combine(juce::dsp::AudioBlock<FloatType> &block) {
        auto lowBlock = juce::dsp::AudioBlock<FloatType>(lowBuffer);
        auto midBlock = juce::dsp::AudioBlock<FloatType>(midBuffer);
        auto highBlock = juce::dsp::AudioBlock<FloatType>(highBuffer);

        block.replaceWithSumOf(lowBlock, midBlock);
        block.add(highBlock);
    }

    template
    class FIRCrossover<float>;

    template
    class FIRCrossover<double>;
}

