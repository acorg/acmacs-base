#pragma once

#include <cmath>
#include <limits>
#include <iostream>

#include "acmacs-base/float.hh"
#include "acmacs-base/to-string.hh"
#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace acmacs::detail
{
    template <typename Tag> class SizeScale
    {
     public:
        constexpr explicit SizeScale() : mValue{0} {}
        constexpr explicit SizeScale(double aValue) : mValue{aValue} {}
        constexpr explicit SizeScale(int aValue) : mValue{static_cast<double>(aValue)} {}
        SizeScale(bool) = delete;
        constexpr SizeScale(const SizeScale& a) = default;
        constexpr bool operator==(SizeScale<Tag> a) const { return float_equal(mValue, a.mValue); }
        constexpr bool operator!=(SizeScale<Tag> a) const { return !operator==(a); }
        constexpr bool operator<(SizeScale<Tag> a) const { return mValue < a.mValue; }
        constexpr SizeScale& operator = (double aValue) { mValue = aValue; return *this; }
        constexpr SizeScale& operator = (const SizeScale& a) = default;
        constexpr double value() const { return mValue; }
        constexpr SizeScale operator / (double a) const { return SizeScale{mValue / a}; }
        constexpr SizeScale operator * (double a) const { return SizeScale{mValue * a}; }
        constexpr SizeScale& operator *= (double a) { mValue *= a; return *this; }
        constexpr SizeScale operator - () const { return SizeScale{- mValue}; }
        constexpr SizeScale operator - (SizeScale<Tag> a) const { return SizeScale{mValue - a.mValue}; }
        constexpr SizeScale operator + (SizeScale<Tag> a) const { return SizeScale{mValue + a.mValue}; }
        constexpr SizeScale& operator += (SizeScale<Tag> a) { mValue += a.mValue; return *this; }
        constexpr bool empty() const { return std::isnan(mValue); }
        static constexpr SizeScale make_empty() { return SizeScale(std::numeric_limits<double>::quiet_NaN()); }

     private:
        double mValue;
    };
}

using Pixels = acmacs::detail::SizeScale<struct Pixels_tag>; // size in pixels, indepenent from the surface internal coordinate system
using Scaled = acmacs::detail::SizeScale<struct Scaled_tag>; // size in the surface internal coordinate system

// ----------------------------------------------------------------------

using Aspect = acmacs::detail::SizeScale<struct Aspect_tag>;
using Rotation = acmacs::detail::SizeScale<struct Rotation_tag>;

constexpr inline Rotation RotationDegrees(double aAngle)
{
    return Rotation{aAngle * M_PI / 180.0};
}

inline Rotation RotationRadiansOrDegrees(double aAngle)
{
    if (std::abs(aAngle) < 3.15)
        return Rotation{aAngle};
    else
        return RotationDegrees(aAngle);
}

constexpr const Rotation NoRotation{0.0};
constexpr const Rotation RotationReassortant{0.5};
constexpr const Rotation Rotation90DegreesClockwise{RotationDegrees(90)};
constexpr const Rotation Rotation90DegreesAnticlockwise{RotationDegrees(-90)};
constexpr const Aspect AspectNormal{1.0};
constexpr const Aspect AspectEgg{0.75};

// ----------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& out, Pixels aPixels) { return out << "Pixels{" << aPixels.value() << '}'; }
inline std::ostream& operator<<(std::ostream& out, Scaled aScaled) { return out << "Scaled{" << aScaled.value() << '}'; }
inline std::ostream& operator<<(std::ostream& out, Aspect aAspect) { if (aAspect == AspectNormal) return out << "AspectNormal"; else return out << "Aspect{" << aAspect.value() << '}'; }
inline std::ostream& operator<<(std::ostream& out, Rotation aRotation) { if (aRotation == NoRotation) return out << "NoRotation"; else return out << "Rotation{" << aRotation.value() << '}'; }

namespace acmacs
{
    inline std::string to_string(Aspect aAspect) { return aAspect == AspectNormal ? std::string{"1.0"} : to_string(aAspect.value()); }
    inline std::string to_string(Rotation aRotation) { return aRotation == NoRotation ? std::string{"0.0"} : to_string(aRotation.value()); }
    inline std::string to_string(Pixels aPixels) { return to_string(aPixels.value()); }
    inline std::string to_string(Scaled aScaled) { return to_string(aScaled.value()); }

} // namespace acmacs

namespace acmacs::detail
{
    using ::operator<<;

} // namespace acmacs::detail

// ----------------------------------------------------------------------

template <> struct fmt::formatter<Pixels> : public fmt::formatter<acmacs::fmt_default_formatter>
{
    template <typename FormatContext> auto format(const Pixels& pixels, FormatContext& ctx) { return format_to(ctx.out(), "{}px", pixels.value()); }
};


template <> struct fmt::formatter<Scaled> : public fmt::formatter<acmacs::fmt_default_formatter>
{
    template <typename FormatContext> auto format(const Scaled& scaled, FormatContext& ctx) { return format_to(ctx.out(), "{}scaled", scaled.value()); }
};

template <> struct fmt::formatter<Rotation> : public fmt::formatter<acmacs::fmt_default_formatter>
{
    template <typename FormatContext> auto format(const Rotation& rotation, FormatContext& ctx) { return format_to(ctx.out(), "{}", rotation.value()); }
};

template <> struct fmt::formatter<Aspect> : public fmt::formatter<acmacs::fmt_default_formatter>
{
    template <typename FormatContext> auto format(const Aspect& aspect, FormatContext& ctx) { return format_to(ctx.out(), "{}", aspect.value()); }
};


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
