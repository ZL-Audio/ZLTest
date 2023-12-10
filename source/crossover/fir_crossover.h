//
// Created by Zishu Liu on 12/9/23.
//

#ifndef ZLSPLIT_CROSSOVER_H
#define ZLSPLIT_CROSSOVER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zl_crossover {
    template<typename FloatType>
    class FIRCrossover {
    public:
        FIRCrossover(juce::AudioProcessor &processorRef, size_t filterOrder);

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

        inline int getLatency() {
            return static_cast<int>((order) / 2);
        }

    private:
        juce::AudioProcessor *p;
        size_t order = 0;
        juce::dsp::ProcessSpec filterSpec;
        FloatType lowFreq = 240, highFreq = 4800;
        juce::dsp::ProcessorDuplicator<
                juce::dsp::FIR::Filter<FloatType>,
                juce::dsp::FIR::Coefficients<FloatType>> lowPass, lowMidPass;
        juce::dsp::DelayLine<FloatType> allPass;
        juce::dsp::Gain<FloatType> lowGain, lowMidGain;
        juce::AudioBuffer<FloatType> lowBuffer, midBuffer, highBuffer;
    };
}


#endif //ZLSPLIT_CROSSOVER_H
