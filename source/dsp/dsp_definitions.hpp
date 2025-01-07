// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEQUALIZER_DSP_DEFINITIONS_HPP
#define ZLEQUALIZER_DSP_DEFINITIONS_HPP

#include <juce_audio_processors/juce_audio_processors.h>

namespace zlDSP {
    inline auto static constexpr versionHint = 1;

    template<typename FloatType>
    inline juce::NormalisableRange<FloatType> getLogMidRange(
        const FloatType xMin, const FloatType xMax, const FloatType xMid, const FloatType xInterval) {
        const FloatType rng1{std::log(xMid / xMin) * FloatType(2)};
        const FloatType rng2{std::log(xMax / xMid) * FloatType(2)};
        return {
            xMin, xMax,
            [=](FloatType, FloatType, const FloatType v) {
                return v < FloatType(.5) ? std::exp(v * rng1) * xMin : std::exp((v - FloatType(.5)) * rng2) * xMid;
            },
            [=](FloatType, FloatType, const FloatType v) {
                return v < xMid ? std::log(v / xMin) / rng1 : FloatType(.5) + std::log(v / xMid) / rng2;
            },
            [=](FloatType, FloatType, const FloatType v) {
                const FloatType x = xMin + xInterval * std::round ((v - xMin) / xInterval);
                return x <= xMin ? xMin : (x >= xMax ? xMax : x);
            }
        };
    }

    inline juce::NormalisableRange<double> logMidRange(
        const double xMin, const double xMax, const double xMid, const double xInterval) {
        return getLogMidRange<double>(xMin, xMax, xMid, xInterval);
    }

    inline juce::NormalisableRange<float> logMidRange(
        const float xMin, const float xMax, const float xMid, const float xInterval) {
        return getLogMidRange<float>(xMin, xMax, xMid, xInterval);
    }

    // float
    template<class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::ID + suffix, versionHint),
                                                               T::name + suffix, T::range, T::defaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterFloat> get(bool meta, const std::string &suffix = "",
                                                              bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::name).
                    withMeta(meta);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::ID + suffix, versionHint),
                                                               T::name + suffix, T::range, T::defaultV, attributes);
        }

        inline static float convertTo01(const float x) {
            return T::range.convertTo0to1(x);
        }
    };

    // bool
    template<class T>
    class BoolParameters {
    public:
        static std::unique_ptr<juce::AudioParameterBool> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::ID + suffix, versionHint),
                                                              T::name + suffix, T::defaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterBool> get(bool meta, const std::string &suffix = "",
                                                             bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::name).
                    withMeta(meta);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::ID + suffix, versionHint),
                                                              T::name + suffix, T::defaultV, attributes);
        }

        inline static float convertTo01(const bool x) {
            return x ? 1.f : 0.f;
        }
    };

    // choice
    template<class T>
    class ChoiceParameters {
    public:
        static std::unique_ptr<juce::AudioParameterChoice> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::ID + suffix, versionHint),
                                                                T::name + suffix, T::choices, T::defaultI, attributes);
        }

        static std::unique_ptr<juce::AudioParameterChoice> get(bool meta, const std::string &suffix = "",
                                                               bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::name).
                    withMeta(meta);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::ID + suffix, versionHint),
                                                                T::name + suffix, T::choices, T::defaultI, attributes);
        }

        inline static float convertTo01(const int x) {
            return static_cast<float>(x) / static_cast<float>(T::choices.size() - 1);
        }
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        // layout.add();
        return layout;
    }

    inline void updateParaNotifyHost(juce::RangedAudioParameter *para, float value) {
        para->beginChangeGesture();
        para->setValueNotifyingHost(value);
        para->endChangeGesture();
    }
}

#endif //ZLEQUALIZER_DSP_DEFINITIONS_HPP
