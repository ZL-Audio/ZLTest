#pragma once

#include "over_sample_stage.hpp"
#include "halfband_coeffs.hpp"

namespace zldsp::oversample {
    template<typename FloatType, size_t NumStage>
    class OverSampler {
    private:
        static constexpr std::array kCoeff_100_06_90 = halfband_coeff::convert<FloatType>(
            halfband_coeff::kCoeff_100_06_90);
        static constexpr std::array kCoeff_64_08_80 = halfband_coeff::convert<FloatType>(
            halfband_coeff::kCoeff_64_08_80);
        static constexpr std::array kCoeff_32_20_80 = halfband_coeff::convert<FloatType>(
            halfband_coeff::kCoeff_32_20_80);

    public:
        OverSampler() {
            static_assert(NumStage >= 1);
            static_assert(NumStage <= 6);
            stages_.emplace_back(OverSampleStage<FloatType>{
                std::span(kCoeff_100_06_90),
                std::span(kCoeff_100_06_90)
            });
            for (size_t i = 1; i < NumStage; ++i) {
                stages_.emplace_back(OverSampleStage<FloatType>{
                    std::span(kCoeff_32_20_80),
                    std::span(kCoeff_32_20_80)
                });
            }
        }

        void prepare(const size_t num_channels, const size_t num_samples) {
            auto stage_num_samples = num_samples;
            for (size_t i = 0; i < NumStage; ++i) {
                stages_[i].prepare(num_channels, stage_num_samples);
                stage_num_samples = stage_num_samples << 1;
            }
            pointers.resize(num_channels);
        }

        void reset() {
            for (auto &stage: stages_) {
                stage.reset();
            }
        }

        [[nodiscard]] size_t getLatency() const {
            size_t total_latency{0};
            for (size_t i = 0; i < NumStage; ++i) {
                total_latency += stages_[i].getLatency() >> i;
            }
            return total_latency;
        }

        void upsample(std::span<FloatType *> buffer, const size_t num_samples) {
            auto stage_num_sample = num_samples;
            stages_[0].upsample(buffer, stage_num_sample);
            for (size_t i = 1; i < NumStage; ++i) {
                stage_num_sample = stage_num_sample << 1;
                stages_[i].upsample(stages_[i - 1].getOSPointer(), stage_num_sample);
            }
        }

        void downsample(std::span<FloatType *> buffer, const size_t num_samples) {
            auto stage_num_sample = num_samples << (NumStage - 1);
            for (size_t i = NumStage - 1; i > 0; --i) {
                stages_[i].downsample(stages_[i - 1].getOSPointer(), stage_num_sample);
                stage_num_sample = stage_num_sample >> 1;
            }
            stages_[0].downsample(buffer, num_samples);
        }

        std::vector<std::vector<FloatType> > &getOSBuffer() {
            return stages_.back().getOSBuffer();
        }

        std::vector<FloatType *> &getOSPointer() {
            return stages_.back().getOSPointer();
        }

    private:
        std::vector<OverSampleStage<FloatType> > stages_;
        std::vector<FloatType *> pointers;
    };
}
