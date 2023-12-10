//
// Created by Zishu Liu on 12/10/23.
//

#ifndef ZLTEST_LR_FILTER_H
#define ZLTEST_LR_FILTER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zl_crossover {
    template<typename FloatType>
    class LRFilters {
    public:
        LRFilters() {
            filters[0].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
            filters[1].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
            filters[2].setType(juce::dsp::LinkwitzRileyFilterType::allpass);
        }

        void setCutoffFrequency(FloatType freq) {
            for (auto &f: filters) {
                f.setCutoffFrequency(freq);
            }
        }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            for (auto &f: filters) {
                f.prepare(spec);
            }
        }

        void processLow(juce::dsp::AudioBlock<FloatType> block) {
            filters[0].process(juce::dsp::ProcessContextReplacing<FloatType>(block));
        }

        void processHigh(juce::dsp::AudioBlock<FloatType> block) {
            filters[1].process(juce::dsp::ProcessContextReplacing<FloatType>(block));
        }

        void processAll(juce::dsp::AudioBlock<FloatType> block) {
            filters[2].process(juce::dsp::ProcessContextReplacing<FloatType>(block));
        }

    private:
        std::array<juce::dsp::LinkwitzRileyFilter<FloatType>, 3> filters{};
        juce::dsp::LinkwitzRileyFilter<FloatType> low, high, all;
    };
}

#endif //ZLTEST_LR_FILTER_H
