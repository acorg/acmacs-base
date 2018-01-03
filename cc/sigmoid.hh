#pragma once

#include <cmath>

// ----------------------------------------------------------------------

namespace acmacs
{
      // http://en.wikipedia.org/wiki/Sigmoid_function
      // g(aValue) = 1/(1 + exp(-aValue * aPower))
    template <typename Float> inline Float sigmoid(Float aValue, Float aPower)
    {
        return Float{1} / (Float{1} + std::exp(Float{-1} * aValue * aPower));
    }

      // sigmoid derivative
      // Alan uses: dSigmoid = sigmoid*(1-sigmoid)
      // Derek uses: dSigmoid = sigmoid * sigmoid * e^(-x)  [both variants are the same]
      // http://atlas.web.cern.ch/Atlas/GROUPS/SOFTWARE/INFO/Workshops/9905/slides/thu.7/sld007.htm suggests dSigmoid = sigmoid / (1 + e^x) which is slower but more accurate for big x
    template <typename Float> inline Float d_sigmoid(Float aValue, Float aPower)
    {
        const auto s = sigmoid(aValue, aPower);
        return s * (Float{1} - s);
    }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
