#include "acmacs-base/debug.hh"
#include "acmacs-base/color.hh"

#include "color-names.icc"

// ----------------------------------------------------------------------

void Color::from_string(std::string_view src)
{
    try {
        if (src.empty())
            throw std::exception{};
        if (src[0] == '#') {
            const auto v = static_cast<uint32_t>(std::strtoul(src.data() + 1, nullptr, 16));
            switch (src.size()) {
                case 4: // web color #abc -> #aabbcc
                    color_ = ((v & 0xF00) << 12) | ((v & 0xF00) << 8) | ((v & 0x0F0) << 8) | ((v & 0x0F0) << 4) | ((v & 0x00F) << 4) | (v & 0x00F);
                    break;
                case 7:
                case 9:
                    color_ = v;
                    break;
                default:
                    throw std::exception{};
            }
        }
        else {
            color_ = find_color_by_name(src); // color-names.icc, throws if not found
        }
    }
    catch (std::exception&) {
        throw acmacs::color::error{fmt::format("cannot read Color from \"{}\"", src)};
    }

} // Color::from_string

// ----------------------------------------------------------------------

struct HSV
{
    HSV(Color src)
    {
        const double min = std::min({src.red(), src.green(), src.blue()});
        const double max = std::max({src.red(), src.green(), src.blue()});
        const double delta = max - min;

        v = max;
        if (delta < 0.00001) {
            s = 0;
            h = 0; // undefined, maybe nan?
        }
        else if (max > 0.0) {                           // NOTE: if Max is == 0, this divide would cause a crash
            s = delta / max;                            // s
            if (src.red() >= max) {                     // > is bogus, just keeps compilor happy
                h = (src.green() - src.blue()) / delta; // between yellow & magenta
            }
            else {
                if (src.green() >= max)
                    h = 2.0 + (src.blue() - src.red()) / delta; // between cyan & yellow
                else
                    h = 4.0 + (src.red() - src.green()) / delta; // between magenta & cyan
            }
            h *= 60.0; // degrees
            if (h < 0.0)
                h += 360.0;
        }
        else {
            // if max is 0, then r = g = b = 0
            // s = 0, v is undefined
            s = 0.0;
            h = 0.0; // NAN;                            // its now undefined
        }
    }

    constexpr void light(double value) noexcept { s /= value; }
    void adjust_saturation(double value) noexcept { s *= std::abs(value); if (s > 1.0) s = 1.0; }
    void adjust_brightness(double value) noexcept { v *= std::abs(value); if (v > 1.0) v = 1.0; }

    void to_rgb(uint32_t& dest) const noexcept
    {
        const auto from_rgb = [](double r, double g, double b) -> uint32_t { return (uint32_t(r * 255) << 16) | (uint32_t(g * 255) << 8) | uint32_t(b * 255); };

        if (s <= 0.0) { // < is bogus, just shuts up warnings
            dest = from_rgb(v, v, v);
        }
        else {
            const double hh = (h > 360.0 ? 0.0 : h) / 60.0;
            const double ff = hh - std::ceil(hh);
            const double p = v * (1.0 - s);
            const double q = v * (1.0 - (s * ff));
            const double t = v * (1.0 - (s * (1.0 - ff)));

            switch (static_cast<long>(hh)) {
                case 0:
                    dest = from_rgb(v, t, p);
                    break;
                case 1:
                    dest = from_rgb(q, v, p);
                    break;
                case 2:
                    dest = from_rgb(p, v, t);
                    break;
                case 3:
                    dest = from_rgb(p, q, v);
                    break;
                case 4:
                    dest = from_rgb(t, p, v);
                    break;
                default:
                    dest = from_rgb(v, p, q);
                    break;
            }
        }
    }

    double h;       // angle in degrees
    double s;       // percent
    double v;       // percent
};

// ----------------------------------------------------------------------

void Color::light(double value)
{
    if (is_opaque()) {
        HSV hsv{color_ == 0 ? RED : *this}; // special case for black
        hsv.light(value);
        hsv.to_rgb(color_);
    }

} // Color::light

// ----------------------------------------------------------------------

void Color::adjust_saturation(double value)
{
    if (is_opaque()) {
        HSV hsv{*this};
        hsv.adjust_saturation(value);
        hsv.to_rgb(color_);
    }

} // Color::adjust_saturation

// ----------------------------------------------------------------------

void Color::adjust_brightness(double value)
{
    if (is_opaque()) {
        HSV hsv{*this};
        hsv.adjust_brightness(value);
        hsv.to_rgb(color_);
    }

} // Color::adjust_brightness

// ----------------------------------------------------------------------

void Color::adjust_transparency(double value)
{
    if (value < 0.0)
        throw acmacs::color::error{fmt::format("Color::adjust_transparency: attempt to use negative value: {}", value)};
    const auto transparency = (1.0 - alpha()) * value;
    if (transparency >= 1.0)
        set_transparency(1.0);
    else
        set_transparency(transparency);

} // Color::adjust_transparency

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
