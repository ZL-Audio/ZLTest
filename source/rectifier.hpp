// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "adaa2.hpp"

namespace zldsp::compressor {
    template<typename FloatType>
    class Rectifier : public zldsp::adaa::ADAA2<FloatType> {
    public:
        Rectifier() = default;

    private:
        FloatType g0(FloatType x) override {
            return std::abs(x);
        }

        FloatType g1(FloatType x) override {
            return FloatType(0.5) * x * std::abs(x);
        }

        FloatType g2(FloatType x) override {
            return std::abs(FloatType(1) / FloatType(6) * x * x * x);
        }
    };
}
