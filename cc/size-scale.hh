#pragma once

#include <cmath>
#include <limits>

#include "float.hh"

// ----------------------------------------------------------------------

namespace _acmacs_base_internal
{
    template <char Tag> class SizeScale
    {
     public:
        inline SizeScale() : mValue(0) {}
        inline explicit SizeScale(double aValue) : mValue(aValue) {}
          // inline SizeScale(const SizeScale& a) = default;
        inline bool operator==(SizeScale a) const { return float_equal(mValue, a.mValue); }
        inline SizeScale& operator = (double aValue) { mValue = aValue; return *this; }
        inline double value() const { return mValue; }
        inline SizeScale operator / (double a) const { return SizeScale{mValue / a}; }
        inline SizeScale operator * (double a) const { return SizeScale{mValue * a}; }
        inline SizeScale& operator *= (double a) { mValue *= a; return *this; }
        inline SizeScale operator - () const { return SizeScale{- mValue}; }
        inline SizeScale operator - (const SizeScale& a) const { return SizeScale{mValue - a.mValue}; }
        inline SizeScale operator + (const SizeScale& a) const { return SizeScale{mValue + a.mValue}; }
        inline bool empty() const { return std::isnan(mValue); }
        static SizeScale make_empty() { return SizeScale(std::numeric_limits<double>::quiet_NaN()); }

     private:
        double mValue;
    };
}

using Pixels = _acmacs_base_internal::SizeScale<'P'>; // size in pixels, indepenent from the surface internal coordinate system
using Scaled = _acmacs_base_internal::SizeScale<'S'>; // size in the surface internal coordinate system

// ----------------------------------------------------------------------

using Aspect = _acmacs_base_internal::SizeScale<'A'>;
using Rotation = _acmacs_base_internal::SizeScale<'R'>;

#include "acmacs-base/global-constructors-push.hh"

const Rotation NoRotation{0.0};
const Aspect AspectNormal{1.0};

#include "acmacs-base/diagnostics-pop.hh"

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End: