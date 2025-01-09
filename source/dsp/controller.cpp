// Copyright (C) 2025 - zsliu98
// This file is part of ZLDuckerTest
//
// ZLDuckerTest is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLDuckerTest is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLDuckerTest. If not, see <https://www.gnu.org/licenses/>.

#include "controller.hpp"

namespace zlDSP {
    template<typename FloatType>
    Controller<FloatType>::Controller(juce::AudioProcessor &processor) : processorRef(processor) {
        for (size_t i = 0; i < FilterNum; ++i) {
            duckerFilters[i].setFreq(static_cast<FloatType>(duckerPos[i]));
            duckerFilters[i].setFilterType(zlFilter::FilterType::peak);
            mainPassFilters[i].setFreq(static_cast<FloatType>(duckerPos[i]));
            mainPassFilters[i].setQ(FloatType(2.8284271247461903 * 2.0));
            mainPassFilters[i].setFilterType(zlFilter::FilterType::bandPass);
            auxPassFilters[i].setFreq(static_cast<FloatType>(duckerPos[i]));
            auxPassFilters[i].setQ(FloatType(2.8284271247461903 * 2.0));
            auxPassFilters[i].setFilterType(zlFilter::FilterType::bandPass);

            currentGains[i] = FloatType(0);
        }
        setStrength(FloatType(0.5));
        setSmooth(FloatType(0.5));
    }

    template<typename FloatType>
    void Controller<FloatType>::reset() {
        for (size_t i = 0; i < FilterNum; ++i) {
            duckerFilters[i].setToRest();
            mainPassFilters[i].setToRest();
            auxPassFilters[i].setToRest();
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        for (size_t i = 0; i < FilterNum; ++i) {
            duckerFilters[i].prepare(spec);
            mainPassFilters[i].prepare(spec);
            auxPassFilters[i].prepare(spec);
        }
        samplePerBuffer = static_cast<int>(spec.sampleRate * 0.001);
        passBuffer.setSize(static_cast<int>(spec.numChannels), samplePerBuffer);
        for (size_t i = 0; i < FilterNum; ++i) {
            duckerFilters[i].processPre(passBuffer);
            mainPassFilters[i].processPre(passBuffer);
            auxPassFilters[i].processPre(passBuffer);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        currentUseSide = useSide.load();
        currentStrength = strength.load();
        currentRange = range.load();
        if (toUpdateFocus.exchange(false)) {
            const auto currentFocus = focus.load();
            const auto currentQ = currentFocus < FloatType(0.5)
                                      ? std::pow(FloatType(64), currentFocus)
                                      : FloatType(32) * currentFocus - FloatType(8);
            for (size_t i = 0; i < FilterNum; ++i) {
                mainPassFilters[i].template setQSync<true>(currentQ);
                auxPassFilters[i].template setQSync<true>(currentQ);
            }
        }
        int startSample = 0;
        while (startSample < buffer.getNumSamples()) {
            const int actualNumSample = std::min(samplePerBuffer, buffer.getNumSamples() - startSample);
            auto subBuffer = juce::AudioBuffer<FloatType>(buffer.getArrayOfWritePointers(),
                                                          4, startSample, actualNumSample);
            processSubBuffer(subBuffer);
            startSample += samplePerBuffer;
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::processSubBuffer(juce::AudioBuffer<FloatType> &buffer) {
        const auto bufferRelSize = static_cast<FloatType>(buffer.getNumSamples()) / static_cast<FloatType>(samplePerBuffer);
        // calculate main RMS
        for (size_t i = 0; i < FilterNum; ++i) {
            juce::FloatVectorOperations::copy(passBuffer.getWritePointer(0),
                                              buffer.getReadPointer(0),
                                              buffer.getNumSamples());
            juce::FloatVectorOperations::copy(passBuffer.getWritePointer(1),
                                              buffer.getReadPointer(1),
                                              buffer.getNumSamples());
            mainPassFilters[i].process(passBuffer);
            const auto leftRMS = passBuffer.getRMSLevel(0, 0, buffer.getNumSamples());
            const auto rightRMS = passBuffer.getRMSLevel(1, 0, buffer.getNumSamples());
            mainPassRMS[i] = juce::Decibels::gainToDecibels((leftRMS + rightRMS) * FloatType(0.5));
        }
        // calculate aux RMS
        if (currentUseSide) {
            for (size_t i = 0; i < FilterNum; ++i) {
                juce::FloatVectorOperations::copy(passBuffer.getWritePointer(0),
                                                  buffer.getReadPointer(2),
                                                  buffer.getNumSamples());
                juce::FloatVectorOperations::copy(passBuffer.getWritePointer(1),
                                                  buffer.getReadPointer(3),
                                                  buffer.getNumSamples());
                auxPassFilters[i].process(passBuffer);
                const auto leftRMS = passBuffer.getRMSLevel(0, 0, buffer.getNumSamples());
                const auto rightRMS = passBuffer.getRMSLevel(1, 0, buffer.getNumSamples());
                auxPassRMS[i] = juce::Decibels::gainToDecibels((leftRMS + rightRMS) * FloatType(0.5));
            }
        } else {
            for (size_t i = 0; i < FilterNum; ++i) {
                auxPassRMS[i] = mainPassRMS[i];
            }
        }
        // calculate collision threshold
        const auto mainM = std::reduce(
                               mainPassRMS.begin(), mainPassRMS.end()) / static_cast<FloatType>(mainPassRMS.size());
        const auto refM = std::reduce(
                              auxPassRMS.begin(), auxPassRMS.end()) / static_cast<FloatType>(auxPassRMS.size());
        const auto threshold = juce::jmin(currentStrength * (mainM + refM), FloatType(0));
        // calculate delta gains
        dynamicIndices.clear();
        staticIndices.clear();
        const auto maximumShift1 = FloatType(0.025) * bufferRelSize;
        const auto maximumShift2 = FloatType(0.0025) * bufferRelSize;
        for (size_t i = 0; i < FilterNum; ++i) {
            const auto rmsMin = std::min(mainPassRMS[i], auxPassRMS[i]);
            auto targetGain = rmsMin < threshold
                                  ? FloatType(0)
                                  : (rmsMin - threshold) / (FloatType(0.01) - threshold) * (-currentRange);
            targetGain = std::clamp(targetGain, currentGains[i] - maximumShift1, currentGains[i] + maximumShift2);
            deltaGains[i] = (targetGain - currentGains[i]) / static_cast<FloatType>(buffer.getNumSamples());
            if (std::abs(targetGain - currentGains[i]) > FloatType(1e-6)) {
                dynamicIndices.push(i);
            } else {
                staticIndices.push(i);
            }
        }
        // process ducking filters
        auto audioWriters = buffer.getArrayOfWritePointers();
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            sampleBuffer.setDataToReferTo(audioWriters, 2, i, 1);
            for (size_t tidx = 0; tidx < dynamicIndices.size(); ++tidx) {
                const auto idx = dynamicIndices[tidx];
                currentGains[idx] += deltaGains[idx];
                duckerFilters[idx].template setGainSync<true>(currentGains[idx]);
                duckerFilters[idx].template process<false>(sampleBuffer);
            }
        }
        // process static filters
        sampleBuffer.setDataToReferTo(audioWriters, 2, 0, buffer.getNumSamples());
        for (size_t tidx = 0; tidx < staticIndices.size(); ++tidx) {
            const auto idx = staticIndices[tidx];
            duckerFilters[idx].template process<false>(sampleBuffer);
        }
    }

    template
    class Controller<float>;

    template
    class Controller<double>;
} // zlDSP
