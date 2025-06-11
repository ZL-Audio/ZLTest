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
            up_coeff_center_pos_ = up_coeff_.size() / 2;

            down_coeff_.resize(down_coeff.size());
            for (size_t i = 0; i < down_coeff.size(); ++i) {
                down_coeff_[i] = down_coeff[i];
            }
            latency_ = (up_coeff.size() + down_coeff.size() - 2) / 4;
        }

        void prepare(const size_t num_channels, const size_t num_samples) {
            up_delay_lines_.resize(num_channels, kfr::univector<FloatType>(up_coeff_.size(), FloatType(0)));
            down_delay_lines_.resize(num_channels, kfr::univector<FloatType>(down_coeff_.size(), FloatType(0)));
            os_buffers_.resize(num_channels, std::vector<FloatType>(num_samples << 1));
            os_pointers_.resize(num_channels);
            for (size_t chan = 0; chan < num_channels; ++chan) {
                os_pointers_[chan] = os_buffers_[chan].data();
            }
            reset();
        }

        void reset() {
            up_cursor_ = 0;
            down_cursor_ = 0;
            for (auto &d: up_delay_lines_) {
                std::fill(d.begin(), d.end(), FloatType(0));
            }
            for (auto &d: down_delay_lines_) {
                std::fill(d.begin(), d.end(), FloatType(0));
            }
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
                    os_data[i << 1] = delay_line[up_coeff_center_pos_] * up_coeff_center_;
                    std::rotate(delay_line.begin(), delay_line.begin() + 1, delay_line.end());
                    delay_line[delay_line.size() - 1] = chan_data[i];
                    os_data[(i << 1) + 1] = kfr::dotproduct(up_coeff_, delay_line);
                }
            }
        }

        void downsample(std::span<FloatType *> buffer, const size_t num_samples) {
            for (size_t chan = 0; chan < buffer.size(); ++chan) {
                auto &delay_line{down_delay_lines_[chan]};
                auto chan_data{buffer[chan]};
                auto os_data{os_buffers_[chan].data()};
                auto cursor = down_cursor_;
                for (size_t i = 0; i < num_samples; ++i) {
                    delay_line.ringbuf_write(cursor, os_data[i << 1]);
                    FloatType v = kfr::dotproduct(down_coeff_.slice(0, down_coeff_.size() - cursor),
                                                  delay_line.slice(cursor));
                    if (cursor > 0) {
                        v += kfr::dotproduct(down_coeff_.slice(down_coeff_.size() - cursor),
                                             delay_line.slice(0, cursor));
                    }
                    delay_line.ringbuf_write(cursor, os_data[(i << 1) + 1]);
                    chan_data[i] = v;
                }
            }
            down_cursor_ = (down_cursor_ + 2 * num_samples) % down_coeff_.size();
        }

        std::vector<std::vector<FloatType> > &getOSBuffer() {
            return os_buffers_;
        }

        std::vector<FloatType *> &getOSPointer() {
            return os_pointers_;
        }

    private:
        size_t up_cursor_{0};
        kfr::univector<FloatType> up_coeff_{};
        FloatType up_coeff_center_{FloatType(0)};
        size_t up_coeff_center_pos_{0};
        std::vector<kfr::univector<FloatType> > up_delay_lines_{};

        size_t down_cursor_{0};
        kfr::univector<FloatType> down_coeff_{};
        std::vector<kfr::univector<FloatType> > down_delay_lines_{};

        size_t latency_{0};

        std::vector<std::vector<FloatType> > os_buffers_{};
        std::vector<FloatType *> os_pointers_{};
    };
}
