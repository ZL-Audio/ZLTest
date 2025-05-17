// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <vector>
#include <array>
#include <algorithm>

#include "adaa2.hpp"

namespace zldsp::compressor {
    template<typename FloatType>
    struct LinearCurve {
        FloatType b, c;

        void setPara(FloatType t, FloatType r, FloatType) {
            b = FloatType(1) / r;
            c = t * (FloatType(1) - FloatType(1) / r);
        }
    };

    template<typename FloatType>
    struct DownCurve {
        FloatType a, c;
        static constexpr FloatType b{FloatType(0)};

        void setPara(FloatType t, FloatType r, FloatType w) {
            a = FloatType(0.5) / (r * std::min(t + w, FloatType(-0.0001)));
            c = FloatType(0.5) * (w - t) / r + t;
        }
    };

    template<typename FloatType>
    struct UpCurve {
        FloatType a, c;
        static constexpr FloatType b{FloatType(1)};

        void setPara(FloatType t, FloatType r, FloatType w) {
            a = FloatType(0.5) * (FloatType(1) - r) / (r * std::min(t + w, FloatType(-0.0001)));
            c = FloatType(0.5) * (FloatType(1) - r) * (w - t) / r;
        }
    };

    /**
     * a computer that computes the current compression
     * @tparam FloatType
     */
    template<typename FloatType>
    class KneeComputer final : private zldsp::adaa::ADAA2<FloatType> {
    public:
        KneeComputer() = default;

        bool prepareBuffer() {
            if (to_interpolate_.exchange(false)) {
                interpolate();
                return true;
            }
            return false;
        }

        void copyFrom(KneeComputer &other) {
            low_th_ = other.low_th_;
            high_th_ = other.high_th_;
            para_mid_g0_ = other.para_mid_g0_;
            para_high_g0_ = other.para_high_g0_;
            para_low_g1_ = other.para_low_g1_;
            para_mid_g1_ = other.para_mid_g1_;
            para_high_g1_ = other.para_high_g1_;
            para_low_g2_ = other.para_low_g2_;
            para_mid_g2_ = other.para_mid_g2_;
            para_high_g2_ = other.para_high_g2_;
        }

        template<bool CurrentADAA = false>
        FloatType eval(FloatType x) {
            return zldsp::adaa::ADAA2<FloatType>::template processADAA<CurrentADAA>(x);
        }

        /**
         * computes the current compression
         * @param x input level (in dB)
         * @return current compression (in dB)
         */
        FloatType process(FloatType x) {
            return eval(x) - x;
        }

        inline void setThreshold(FloatType v) {
            threshold_.store(v);
            to_interpolate_.store(true);
        }

        inline FloatType getThreshold() const { return threshold_.load(); }

        inline void setRatio(FloatType v) {
            ratio_.store(std::max(FloatType(1), v));
            to_interpolate_.store(true);
        }

        inline FloatType getRatio() const { return ratio_.load(); }

        inline void setKneeW(FloatType v) {
            knee_w_.store(std::max(v, FloatType(0.01)));
            to_interpolate_.store(true);
        }

        inline FloatType getKneeW() const { return knee_w_.load(); }

        inline void setCurve(FloatType v) {
            curve_.store(std::clamp(v, FloatType(-1), FloatType(1)));
            to_interpolate_.store(true);
        }

        inline FloatType getCurve() const { return curve_.load(); }

    private:
        LinearCurve<FloatType> linear_curve_;
        DownCurve<FloatType> down_curve_;
        UpCurve<FloatType> up_curve_;
        std::atomic<FloatType> threshold_{-18}, ratio_{2};
        std::atomic<FloatType> knee_w_{FloatType(0.25)}, curve_{0};
        FloatType low_th_{0}, high_th_{0};
        // function parameters
        std::array<FloatType, 3> para_mid_g0_{}, para_high_g0_{};
        // first-order anti-derivative function parameters
        std::array<FloatType, 2> para_low_g1_{FloatType(0.5), FloatType(0)};
        std::array<FloatType, 4> para_mid_g1_{};
        std::array<FloatType, 3> para_high_g1_{};
        // second-order anti-derivative function parameters
        std::array<FloatType, 3> para_low_g2_{FloatType(1) / FloatType(6), FloatType(0), FloatType(0)};
        std::array<FloatType, 5> para_mid_g2_{};
        std::array<FloatType, 3> para_high_g2_{};

        std::atomic<bool> to_interpolate_{true};

        static FloatType g0_low(const FloatType x) {
            return x;
        }

        inline FloatType g0_mid(const FloatType x) const {
            return (para_mid_g0_[0] * x + para_mid_g0_[1]) * x + para_mid_g0_[2];
        }

        inline FloatType g0_high(const FloatType x) const {
            return (para_high_g0_[0] * x + para_high_g0_[1]) * x + para_high_g0_[2];
        }

        inline FloatType g1_low(const FloatType x) const {
            return para_low_g1_[0] * x * x + para_low_g1_[1];
        }

        inline FloatType g1_mid(const FloatType x) const {
            return ((para_mid_g1_[0] * x + para_mid_g1_[1]) * x + para_mid_g1_[2]) * x + para_mid_g1_[3];
        }

        inline FloatType g1_high(const FloatType x) const {
            return ((para_high_g1_[0] * x + para_high_g1_[1]) * x + para_high_g1_[2]) * x;
        }

        inline FloatType g2_low(const FloatType x) const {
            return (para_low_g2_[0] * x * x + para_low_g2_[1]) * x + para_low_g2_[2];
        }

        inline FloatType g2_mid(const FloatType x) const {
            return (((para_mid_g2_[0] * x + para_mid_g2_[1]) * x + para_mid_g2_[2]
                ) * x + para_mid_g2_[3]) * x + para_mid_g2_[4];
        }

        inline FloatType g2_high(const FloatType x) const {
            return ((para_high_g2_[0] * x + para_high_g2_[1]) * x + para_high_g2_[2]) * x * x;
        }

        FloatType g0(const FloatType x) override {
            if (x < low_th_) {
                return g0_low(x);
            } else if (x > high_th_) {
                return g0_high(x);
            } else {
                return g0_mid(x);
            }
        }

        FloatType g1(const FloatType x) override {
            if (x < low_th_) {
                return g1_low(x);
            } else if (x > high_th_) {
                return g1_high(x);
            } else {
                return g1_mid(x);
            }
        }

        FloatType g2(const FloatType x) override {
            if (x < low_th_) {
                return g2_low(x);
            } else if (x > high_th_) {
                return g2_high(x);
            } else {
                return g2_mid(x);
            }
        }

        void interpolate() {
            const auto currentThreshold = threshold_.load();
            const auto currentKneeW = knee_w_.load();
            const auto currentRatio = ratio_.load();
            const auto currentCurve = curve_.load();
            low_th_ = currentThreshold - currentKneeW;
            high_th_ = currentThreshold + currentKneeW;
            // update mid curve parameters
            {
                const auto a0 = (FloatType(1) / currentRatio - FloatType(1)) / (currentKneeW * FloatType(4));
                const auto a1 = -low_th_;
                para_mid_g0_[0] = a0;
                const auto a0a1 = a0 * a1;
                para_mid_g0_[1] = FloatType(2) * a0a1 + FloatType(1);
                para_mid_g0_[2] = a0a1 * a1;
            }
            // update high curve parameters
            if (currentCurve >= FloatType(0)) {
                const auto alpha = FloatType(1) - currentCurve, beta = currentCurve;
                linear_curve_.setPara(currentThreshold, currentRatio, currentKneeW);
                down_curve_.setPara(currentThreshold, currentRatio, currentKneeW);
                para_high_g0_[2] = alpha * linear_curve_.c + beta * down_curve_.c;
                para_high_g0_[1] = alpha * linear_curve_.b + beta * down_curve_.b;
                para_high_g0_[0] = beta * down_curve_.a;
            } else {
                const auto alpha = FloatType(1) + currentCurve, beta = -currentCurve;
                linear_curve_.setPara(currentThreshold, currentRatio, currentKneeW);
                up_curve_.setPara(currentThreshold, currentRatio, currentKneeW);
                para_high_g0_[2] = alpha * linear_curve_.c + beta * up_curve_.c;
                para_high_g0_[1] = alpha * linear_curve_.b + beta * up_curve_.b;
                para_high_g0_[0] = beta * up_curve_.a;
            }

            // update g1 parameters
            para_high_g1_[0] = FloatType(1) / FloatType(3) * para_high_g0_[0];
            para_high_g1_[1] = FloatType(1) / FloatType(2) * para_high_g0_[1];
            para_high_g1_[2] = para_high_g0_[2];

            para_mid_g1_[0] = FloatType(1) / FloatType(3) * para_mid_g0_[0];
            para_mid_g1_[1] = FloatType(1) / FloatType(2) * para_mid_g0_[1];
            para_mid_g1_[2] = para_mid_g0_[2];
            para_mid_g1_[3] = FloatType(0);
            para_mid_g1_[3] = g1_high(high_th_) - g1_mid(high_th_);

            para_low_g1_[1] = FloatType(0);
            para_low_g1_[1] = g1_mid(low_th_) - g1_low(low_th_);

            // update g2 parameters
            para_high_g2_[0] = FloatType(1) / FloatType(4) * para_high_g1_[0];
            para_high_g2_[1] = FloatType(1) / FloatType(3) * para_high_g1_[1];
            para_high_g2_[2] = FloatType(1) / FloatType(2) * para_high_g1_[2];

            para_mid_g2_[0] = FloatType(1) / FloatType(4) * para_mid_g1_[0];
            para_mid_g2_[1] = FloatType(1) / FloatType(3) * para_mid_g1_[1];
            para_mid_g2_[2] = FloatType(1) / FloatType(2) * para_mid_g1_[2];
            para_mid_g2_[3] = para_mid_g1_[3];
            para_mid_g2_[4] = FloatType(0);
            para_mid_g2_[4] = g2_high(high_th_) - g2_mid(high_th_);

            para_low_g2_[1] = para_low_g1_[1];
            para_low_g2_[2] = FloatType(0);
            para_low_g2_[2] = g2_mid(low_th_) - g2_low(low_th_);

            zldsp::adaa::ADAA2<FloatType>::resetADAA();
        }
    };
} // KneeComputer
