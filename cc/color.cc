#include "acmacs-base/debug.hh"
#include "acmacs-base/color-hsv.hh"

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
