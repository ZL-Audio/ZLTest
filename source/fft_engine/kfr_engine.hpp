#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Weverything"
#include <kfr/kfr.h>
#include <kfr/dft.hpp>
#pragma clang diagnostic pop

namespace zlFFTEngine {
    template<typename FloatType>
    class KFREngine {
    public:
        KFREngine() = default;

        void setOrder(const size_t order) {
            fft_size = size_t(1) << order;
            fft_plan = std::make_unique<kfr::dft_plan_real<FloatType> >(fft_size);
            temp_buffer.resize(fft_plan->temp_size);
        }

        void forward(FloatType *in_buffer, std::complex<FloatType> *out_buffer) {
            fft_plan->execute(out_buffer, in_buffer, temp_buffer.data());
            const auto scale = FloatType(0.5) / static_cast<FloatType>(fft_plan->size);
            auto vector = kfr::make_univector(out_buffer, fft_size);
            vector = vector * scale;
        }

        void forward(FloatType *in_buffer, FloatType *float_out_buffer) {
            auto out_buffer = reinterpret_cast<std::complex<FloatType> *>(float_out_buffer);
            forward(in_buffer, out_buffer);
        }

        void backward(std::complex<FloatType> *out_buffer, FloatType *in_buffer) {
            fft_plan->execute(in_buffer, out_buffer, temp_buffer.data());
        }

        void backward(FloatType *float_out_buffer, FloatType *in_buffer) {
            auto out_buffer = reinterpret_cast<std::complex<FloatType> *>(float_out_buffer);
            backward(out_buffer, in_buffer);
        }

    private:
        size_t fft_size;
        std::unique_ptr<kfr::dft_plan_real<FloatType> > fft_plan;
        std::vector<kfr::u8> temp_buffer;
    };
}
