#pragma once

#include "acmacs-base/named-type.hh"

// ----------------------------------------------------------------------

struct Pixels : public acmacs::named_double_t<struct Pixels_tag>
{
    using acmacs::named_double_t<struct Pixels_tag>::named_double_t;
    Pixels() : acmacs::named_double_t<struct Pixels_tag>{0.0} {}
};

template <> struct fmt::formatter<Pixels> : public fmt::formatter<acmacs::named_double_t<struct Pixels_tag>> {};

// ----------------------------------------------------------------------

struct Scaled : public acmacs::named_double_t<struct Scaled_tag>
{
    using acmacs::named_double_t<struct Scaled_tag>::named_double_t;
    Scaled() : acmacs::named_double_t<struct Scaled_tag>{0.0} {}
};

template <> struct fmt::formatter<Scaled> : public fmt::formatter<acmacs::named_double_t<struct Scaled_tag>> {};

// ----------------------------------------------------------------------

struct Rotation : public acmacs::named_double_t<struct Rotation_tag>
{
    using acmacs::named_double_t<struct Rotation_tag>::named_double_t;
};

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

template <> struct fmt::formatter<Rotation> : public fmt::formatter<acmacs::named_double_t<struct Rotation_tag>> {};

// ----------------------------------------------------------------------

struct Aspect : public acmacs::named_double_t<struct Aspect_tag>
{
    using acmacs::named_double_t<struct Aspect_tag>::named_double_t;
};

constexpr const Aspect AspectNormal{1.0};
constexpr const Aspect AspectEgg{0.75};

template <> struct fmt::formatter<Aspect> : public fmt::formatter<acmacs::named_double_t<struct Aspect_tag>> {};

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
