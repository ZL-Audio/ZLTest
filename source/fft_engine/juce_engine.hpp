#pragma once

#include <juce_dsp/juce_dsp.h>

namespace zlFFTEngine {
    template<typename FloatType>
    class JUCEEngine {
    public:
        JUCEEngine() = default;

        void setOrder(const size_t order) {
            fft = std::make_unique<juce::dsp::FFT>(static_cast<int>(order));
            fft_size = static_cast<size_t>(1) << order;
        }

        void forward(FloatType *in_buffer, std::complex<FloatType> *out_buffer) {
            auto f_out = reinterpret_cast<FloatType *>(out_buffer);
            memcpy(f_out, in_buffer, fft_size * sizeof(FloatType));
            fft->performRealOnlyForwardTransform(f_out, true);
        }

        void backward(std::complex<FloatType> *out_buffer, FloatType *in_buffer) {
            auto f_out = reinterpret_cast<FloatType *>(out_buffer);
            fft->performRealOnlyInverseTransform(f_out);
            memcpy(in_buffer, f_out, fft_size * sizeof(FloatType));
        }

    private:
        size_t fft_size{0};
        std::unique_ptr<juce::dsp::FFT> fft;
    };
}
