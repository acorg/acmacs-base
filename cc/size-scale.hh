#pragma once

#include <cmath>
#include <limits>

#include "acmacs-base/float.hh"
#include "acmacs-base/to-string.hh"

// ----------------------------------------------------------------------

namespace acmacs::internal
{
    template <char Tag> class SizeScale
    {
     public:
        inline SizeScale() : mValue(0) {}
        inline explicit SizeScale(double aValue) : mValue(aValue) {}
        inline SizeScale(const SizeScale& a) = default;
        constexpr inline bool operator==(SizeScale<Tag> a) const { return float_equal(mValue, a.mValue); }
        constexpr inline bool operator!=(SizeScale<Tag> a) const { return !operator==(a); }
        constexpr inline bool operator<(SizeScale<Tag> a) const { return mValue < a.mValue; }
        inline SizeScale& operator = (double aValue) { mValue = aValue; return *this; }
        inline SizeScale& operator = (const SizeScale& a) = default;
        constexpr inline double value() const { return mValue; }
        constexpr inline SizeScale operator / (double a) const { return SizeScale{mValue / a}; }
        constexpr inline SizeScale operator * (double a) const { return SizeScale{mValue * a}; }
        constexpr inline SizeScale& operator *= (double a) { mValue *= a; return *this; }
        constexpr inline SizeScale operator - () const { return SizeScale{- mValue}; }
        constexpr inline SizeScale operator - (SizeScale<Tag> a) const { return SizeScale{mValue - a.mValue}; }
        constexpr inline SizeScale operator + (SizeScale<Tag> a) const { return SizeScale{mValue + a.mValue}; }
        constexpr inline SizeScale& operator += (SizeScale<Tag> a) { mValue += a.mValue; return *this; }
        constexpr inline bool empty() const { return std::isnan(mValue); }
        static SizeScale make_empty() { return SizeScale(std::numeric_limits<double>::quiet_NaN()); }

     private:
        double mValue;
    };
}

using Pixels = acmacs::internal::SizeScale<'P'>; // size in pixels, indepenent from the surface internal coordinate system
using Scaled = acmacs::internal::SizeScale<'S'>; // size in the surface internal coordinate system

// ----------------------------------------------------------------------

using Aspect = acmacs::internal::SizeScale<'A'>;
using Rotation = acmacs::internal::SizeScale<'R'>;

#include "acmacs-base/global-constructors-push.hh"

const Rotation NoRotation{0.0};
const Rotation RotationReassortant{0.5};
const Aspect AspectNormal{1.0};
const Aspect AspectEgg{0.75};

#include "acmacs-base/diagnostics-pop.hh"

inline Rotation RotationDegrees(double aAngle)
{
    return Rotation{aAngle * M_PI / 180.0};
}

// ----------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& out, Pixels aPixels) { return out << "Pixels{" << aPixels.value() << '}'; }
inline std::ostream& operator<<(std::ostream& out, Scaled aScaled) { return out << "Scaled{" << aScaled.value() << '}'; }
inline std::ostream& operator<<(std::ostream& out, Aspect aAspect) { if (aAspect == AspectNormal) return out << "AspectNormal"; else return out << "Aspect{" << aAspect.value() << '}'; }
inline std::ostream& operator<<(std::ostream& out, Rotation aRotation) { if (aRotation == NoRotation) return out << "NoRotation"; else return out << "Rotation{" << aRotation.value() << '}'; }

namespace acmacs
{
    template <> inline std::string to_string(Aspect aAspect) { return aAspect == AspectNormal ? std::string{"1.0"} : to_string(aAspect.value()); }
    template <> inline std::string to_string(Rotation aRotation) { return aRotation == NoRotation ? std::string{"0.0"} : to_string(aRotation.value()); }
    template <> inline std::string to_string(Pixels aPixels) { return to_string(aPixels.value()); }
    template <> inline std::string to_string(Scaled aScaled) { return to_string(aScaled.value()); }

} // namespace acmacs

namespace acmacs::internal
{
    using ::operator<<;

} // namespace acmacs::internal

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
