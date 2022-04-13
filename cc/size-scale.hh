#pragma once

#include "acmacs-base/named-type.hh"

// ----------------------------------------------------------------------

using Pixels = acmacs::named_double_t<struct Pixels_tag>;
using Scaled = acmacs::named_double_t<struct Scaled_tag>;

// ----------------------------------------------------------------------

using Rotation = acmacs::named_double_t<struct Rotation_tag>;

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

constexpr inline const Rotation NoRotation{0.0};
constexpr inline const Rotation RotationReassortant{0.5};
constexpr inline const Rotation Rotation90DegreesClockwise{RotationDegrees(90)};
constexpr inline const Rotation Rotation90DegreesAnticlockwise{RotationDegrees(-90)};

// ----------------------------------------------------------------------

using Aspect = acmacs::named_double_t<struct Aspect_tag>;

constexpr inline const Aspect AspectNormal{1.0};
constexpr inline const Aspect AspectEgg{0.75};

// ----------------------------------------------------------------------

constexpr Pixels operator-(Pixels rhs) noexcept { return Pixels{-rhs.value()}; }
constexpr Pixels operator/(Pixels lhs, double rhs) noexcept { return Pixels{lhs.value() / rhs}; }
constexpr Pixels operator*(Pixels lhs, double rhs) noexcept { return Pixels{lhs.value() * rhs}; }
constexpr Pixels operator*(Pixels lhs, Aspect rhs) noexcept { return Pixels{lhs.value() * rhs.value()}; }

constexpr Scaled operator-(Scaled rhs) noexcept { return Scaled{-rhs.value()}; }
constexpr Scaled operator/(Scaled lhs, double rhs) noexcept { return Scaled{lhs.value() / rhs}; }
constexpr Scaled operator*(Scaled lhs, double rhs) noexcept { return Scaled{lhs.value() * rhs}; }
constexpr Scaled operator*(Scaled lhs, Aspect rhs) noexcept { return Scaled{lhs.value() * rhs.value()}; }

// ----------------------------------------------------------------------
