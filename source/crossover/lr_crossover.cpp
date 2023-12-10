//
// Created by Zishu Liu on 12/10/23.
//

#include "lr_crossover.h"

namespace zl_crossover {
    template<typename FloatType>
    LRCrossover<FloatType>::LRCrossover(juce::AudioProcessor &processorRef) {
        p = &processorRef;
    }

    template<typename FloatType>
    void LRCrossover<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        lowCross.prepare(spec);
        setLowFreq(lowFreq);
        highCross.prepare(spec);
        setHighFreq(highFreq);

        lowBuffer.setSize(static_cast<int>(spec.numChannels),
                          static_cast<int>(spec.maximumBlockSize));
        midBuffer.setSize(static_cast<int>(spec.numChannels),
                          static_cast<int>(spec.maximumBlockSize));
        highBuffer.setSize(static_cast<int>(spec.numChannels),
                           static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    void LRCrossover<FloatType>::setLowFreq(FloatType freq) {
        lowFreq = freq;
        const juce::GenericScopedLock<juce::CriticalSection> processLock(p->getCallbackLock());

        lowCross.setCutoffFrequency(freq);
    }

    template<typename FloatType>
    void LRCrossover<FloatType>::setHighFreq(FloatType freq) {
        highFreq = freq;
        const juce::GenericScopedLock<juce::CriticalSection> processLock(p->getCallbackLock());

        highCross.setCutoffFrequency(freq);
    }

    template<typename FloatType>
    void LRCrossover<FloatType>::split(juce::dsp::AudioBlock<FloatType> &block) {
        auto lowBlock = juce::dsp::AudioBlock<FloatType>(lowBuffer);
        auto midBlock = juce::dsp::AudioBlock<FloatType>(midBuffer);
        auto highBlock = juce::dsp::AudioBlock<FloatType>(highBuffer);
        lowBlock.copyFrom(block);
        midBlock.copyFrom(block);

        lowCross.processLow(lowBlock);
        highCross.processAll(lowBlock);

        lowCross.processHigh(midBlock);
        highBlock.copyFrom(midBlock);

        highCross.processLow(midBlock);
        highCross.processHigh(highBlock);
    }

    template<typename FloatType>
    void LRCrossover<FloatType>::combine(juce::dsp::AudioBlock<FloatType> &block) {
        auto lowBlock = juce::dsp::AudioBlock<FloatType>(lowBuffer);
        auto midBlock = juce::dsp::AudioBlock<FloatType>(midBuffer);
        auto highBlock = juce::dsp::AudioBlock<FloatType>(highBuffer);

        block.replaceWithSumOf(lowBlock, midBlock);
        block.add(highBlock);
    }

    template
    class LRCrossover<float>;

    template
    class LRCrossover<double>;
}