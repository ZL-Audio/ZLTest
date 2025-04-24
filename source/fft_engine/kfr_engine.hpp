#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#include <kfr/kfr.h>
#include <kfr/dft.hpp>
#pragma clang diagnostic pop

namespace zlFFTEngine {
    template<typename FloatType>
    class KFREngine {
    public:
        KFREngine() = default;

        void setOrder(const size_t order) {
            fft_plan = std::make_unique<kfr::dft_plan_real<FloatType> >(size_t(1) << order);
            temp_buffer.resize(fft_plan->temp_size);
        }

        void forward(FloatType *in_buffer, std::complex<FloatType> *out_buffer) {
            fft_plan->execute(out_buffer, in_buffer, temp_buffer.data());
            const auto scale = FloatType(1) / static_cast<FloatType>(fft_plan->size);
            for (size_t i = 0; i < fft_plan->size; ++i) {
                out_buffer[i].imag(out_buffer[i].imag() * scale);
                out_buffer[i].real(out_buffer[i].real() * scale);
            }
        }

        void backward(std::complex<FloatType> *out_buffer, FloatType *in_buffer) {
            fft_plan->execute(in_buffer, out_buffer, temp_buffer.data());
        }

    private:
        std::unique_ptr<kfr::dft_plan_real<FloatType> > fft_plan;
        std::vector<kfr::u8> temp_buffer;
    };
}
