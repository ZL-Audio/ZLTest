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
    }

    template<typename FloatType>
    void Controller<FloatType>::process(juce::AudioBuffer<FloatType> &buffer) {
        currentUseSide = useSide.load();
        currentStrength = strength.load();
        int startSample = 0;
        for (size_t i = 0; i < FilterNum; ++i) {
            duckerFilters[i].processPre(buffer);
            mainPassFilters[i].processPre(buffer);
            auxPassFilters[i].processPre(buffer);
        }
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
        for (size_t i = 0; i < FilterNum; ++i) {
            const auto rmsMin = std::min(mainPassRMS[i], auxPassRMS[i]);
            const auto targetGain = rmsMin < threshold
                                        ? FloatType(0)
                                        : (rmsMin - threshold) / (FloatType(0.01) - threshold) * (-currentRange);
            // logger.logMessage(
            //     juce::String(mainPassRMS[i]) + " " + juce::String(auxPassRMS[i]) + " " + juce::String(threshold));
            deltaGains[i] = (targetGain - currentGains[i]) / static_cast<FloatType>(buffer.getNumSamples());
            isDucking[i] = std::abs(targetGain - currentGains[i]) > FloatType(0.01);
        }
        // process ducking
        auto audioWriters = buffer.getArrayOfWritePointers();
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            sampleBuffer.setDataToReferTo(audioWriters, 2, i, 1);
            for (size_t idx = 0; idx < FilterNum; ++idx) {
                currentGains[idx] += deltaGains[idx];
                if (isDucking[idx]) {
                    duckerFilters[idx].template setGainSync<true>(currentGains[idx]);
                }
                duckerFilters[idx].template process<false>(sampleBuffer);
            }
        }
    }

    template
    class Controller<float>;

    template
    class Controller<double>;
} // zlDSP
