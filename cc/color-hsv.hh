#pragma once

#include "acmacs-base/color.hh"

// ----------------------------------------------------------------------

namespace acmacs::color
{
    struct HSV
    {
        HSV(Color src);

        uint32_t rgb() const noexcept;

        constexpr HSV& light(double value) noexcept
        {
            s /= value;
            return *this;
        }

        HSV& adjust_saturation(double value) noexcept
        {
            s *= std::abs(value);
            if (s > 1.0)
                s = 1.0;
            return *this;
        }
        HSV& adjust_brightness(double value) noexcept
        {
            v *= std::abs(value);
            if (v > 1.0)
                v = 1.0;
            return *this;
        }

        double h; // angle in degrees 0-360
        double s; // percent
        double v; // percent
    };

} // namespace acmacs::color

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
