#pragma once

#include <cmath>

// ----------------------------------------------------------------------

namespace acmacs
{
      // http://en.wikipedia.org/wiki/Sigmoid_function
      // g(aValue) = 1/(1 + exp(-aValue * aPower))
    template <typename Float> inline Float sigmoid(Float aValue)
    {
        return Float{1} / (Float{1} + std::exp(Float{-1} * aValue));
    }

      // https://groups.google.com/forum/#!topic/comp.ai.neural-nets/gqekclNH3No
    template <typename Float> inline Float sigmoid_fast(Float aValue)
    {
        const Float absx = std::abs(aValue);
        if (absx > static_cast<Float>(8.713655)) {
            return aValue > 0 ? 1 : 0;
        }
        else {
            if (const Float xx = aValue * aValue; absx > Float{4.5})
                return aValue > 0
                        ? (((static_cast<Float>(3.2e-7) * xx - static_cast<Float>(8.544e-5)) * xx + static_cast<Float>(9.99869e-3)) * aValue + static_cast<Float>(0.953157))
                        : (((static_cast<Float>(3.2e-7) * xx - static_cast<Float>(8.544e-5)) * xx + static_cast<Float>(9.99869e-3)) * aValue + static_cast<Float>(0.046843));
            else
                return (((((static_cast<Float>(-5e-8) * xx + static_cast<Float>(3.6e-6)) * xx - static_cast<Float>(1.0621e-4)) * xx + static_cast<Float>(1.75410e-3)) * xx - static_cast<Float>(0.02045660)) * xx + static_cast<Float>(0.24990936)) * aValue + static_cast<Float>(0.499985);
        }
    }

    template <typename Float> inline Float sigmoid_pseudo(Float aValue)
    {
        return aValue > 0 ? 1 : 0;
    }

      // sigmoid derivative
      // Alan uses: dSigmoid = sigmoid*(1-sigmoid)
      // Derek uses: dSigmoid = sigmoid * sigmoid * e^(-x)  [both variants are the same]
      // http://atlas.web.cern.ch/Atlas/GROUPS/SOFTWARE/INFO/Workshops/9905/slides/thu.7/sld007.htm suggests dSigmoid = sigmoid / (1 + e^x) which is slower but more accurate for big x
    template <typename Float> inline Float d_sigmoid(Float aValue)
    {
        const auto s = sigmoid(aValue);
        return s * (Float{1} - s);
    }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
