//
// Created by Zishu Liu on 12/10/23.
//

#ifndef ZLTEST_LR_CROSSOVER_H
#define ZLTEST_LR_CROSSOVER_H

#include "lr_filter.h"

namespace zl_crossover {
    template<typename FloatType>
    class LRCrossover {
    public:
        LRCrossover(juce::AudioProcessor &processorRef);

        void prepare(const juce::dsp::ProcessSpec &spec);

        void split(juce::dsp::AudioBlock<FloatType> &block);

        void combine(juce::dsp::AudioBlock<FloatType> &block);

        inline juce::dsp::AudioBlock<FloatType> getLowBlock() {
            return juce::dsp::AudioBlock<FloatType>(lowBuffer);
        }

        inline juce::dsp::AudioBlock<FloatType> getMidBlock() {
            return juce::dsp::AudioBlock<FloatType>(midBuffer);
        }

        inline juce::dsp::AudioBlock<FloatType> getHighBlock() {
            return juce::dsp::AudioBlock<FloatType>(highBuffer);
        }

        void setLowFreq(FloatType freq);

        void setHighFreq(FloatType freq);

    private:
        juce::AudioProcessor *p;
        juce::dsp::ProcessSpec filterSpec;
        FloatType lowFreq = 240, highFreq = 4800;
        juce::AudioBuffer<FloatType> lowBuffer, midBuffer, highBuffer;
        LRFilters<FloatType> lowCross, highCross;
    };
}

#endif //ZLTEST_LR_CROSSOVER_H
