#pragma once

#include <span>
#include <numbers>
#include <juce_dsp/juce_dsp.h>

#include "halfband_coeffs.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Weverything"
#include <kfr/kfr.h>
#include <kfr/dsp.hpp>
#pragma clang diagnostic pop

namespace zldsp::oversample {
    template<typename FloatType>
    class OverSampleStage {
    public:
        explicit OverSampleStage(std::span<const FloatType> up_coeff,
                                 std::span<const FloatType> down_coeff) {
            up_coeff_.resize(up_coeff.size() / 2);
            for (size_t i = 1; i < up_coeff.size(); i += 2) {
                up_coeff_[i >> 1] = up_coeff[i] * FloatType(2);
            }
            up_coeff_center_ = up_coeff[up_coeff.size() / 2] * FloatType(2);

            down_coeff_.resize(down_coeff.size());
            for (size_t i = 0; i < down_coeff.size(); ++i) {
                down_coeff_[i] = down_coeff[i];
            }

            latency_ = (up_coeff.size() + down_coeff.size() - 2) / 4;
        }

        void prepare(const size_t num_channels, const size_t num_samples) {
            for (size_t i = 0; i < num_channels; ++i) {
                down_filters_.emplace_back(kfr::fir_filter<FloatType>(down_coeff_));
            }
            up_delay_lines_.resize(num_channels, kfr::univector<FloatType>(up_coeff_.size(), FloatType(0)));
            down_delay_lines_.resize(num_channels, kfr::univector<FloatType>(down_coeff_.size(), FloatType(0)));
            os_buffers_.resize(num_channels, std::vector<FloatType>(num_samples << 1));
        }

        [[nodiscard]] size_t getLatency() const {
            return latency_;
        }

        void upsample(std::span<FloatType *> buffer, const size_t num_samples) {
            for (size_t chan = 0; chan < buffer.size(); ++chan) {
                auto &delay_line{up_delay_lines_[chan]};
                auto chan_data{buffer[chan]};
                auto os_data{os_buffers_[chan].data()};
                for (size_t i = 0; i < num_samples; ++i) {
                    os_data[i << 1] = delay_line[delay_line.size() / 2] * up_coeff_center_; // EVEN sample
                    std::rotate(delay_line.begin(), delay_line.begin() + 1, delay_line.end());
                    delay_line[delay_line.size() - 1] = chan_data[i];
                    os_data[(i << 1) + 1] = kfr::dotproduct(up_coeff_, delay_line);
                }
            }
        }

        void downsample(std::span<FloatType *> buffer, const size_t num_samples) {
            for (size_t chan = 0; chan < buffer.size(); ++chan) {
                down_filters_[chan].apply(os_buffers_[chan].data(), num_samples * 2);
                for (size_t idx = 0; idx < num_samples; ++idx) {
                    buffer[chan][idx] = os_buffers_[chan][idx << 1];
                }
            }
        }

        std::vector<std::vector<FloatType> > &getOSBuffer() {
            return os_buffers_;
        }

    private:
        kfr::univector<FloatType> up_coeff_{};
        FloatType up_coeff_center_{FloatType(0)};
        std::vector<kfr::univector<FloatType> > up_delay_lines_{};

        std::vector<FloatType> down_coeff_{};
        std::vector<kfr::univector<FloatType> > down_delay_lines_{};
        std::vector<kfr::fir_filter<FloatType> > down_filters_{};

        size_t latency_{0};

        std::vector<std::vector<FloatType> > os_buffers_{};
    };
}
