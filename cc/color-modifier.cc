#include <cctype>

#include "acmacs-base/color-modifier.hh"
#include "acmacs-base/float.hh"
#include "acmacs-base/string-from-chars.hh"
#include "acmacs-base/string-split.hh"

// ----------------------------------------------------------------------

// see ~/AD/share/doc/color.org
acmacs::color::Modifier::Modifier(std::string_view source)
{
    try {
        for (const auto& field : string::split(source, ":", string::Split::StripRemoveEmpty)) {
            if (field.size() < 2 || field[0] == '#') {
                applicators_.push_back(Color{field});
            }
            else if (std::isdigit(field[1]) || field[1] == '.') {
                const auto value = string::from_chars<double>(field.substr(1));
                if (float_max(value))
                    throw std::exception{};
                switch (field[0]) {
                    case 'h':
                        applicators_.push_back(hue{value});
                        break;
                    case 's':
                        applicators_.push_back(saturation{value});
                        break;
                    case 'b':
                        applicators_.push_back(brightness{value});
                        break;
                    case 't':
                        applicators_.push_back(transparency{value});
                        break;
                    default:
                        throw std::exception{};
                }
            }
        }
    }
    catch (std::exception&) {
        throw acmacs::color::error{fmt::format("cannot read color modifier from \"{}\"", source)};
    }

} // acmacs::color::Modifier::Modifier

// ----------------------------------------------------------------------

acmacs::color::Modifier::operator Color() const
{
    try {
        if (applicators_.size() == 1) {
            return std::visit(
                []<typename Col>(const Col& col) -> Color {
                    if constexpr (std::is_same_v<Col, Color>)
                        return col;
                    else
                        throw std::exception{};
                },
                applicators_.front());
        }
        else
            throw std::exception{};
    }
    catch (std::exception&) {
        throw error{fmt::format("cannot convert acmacs::color::Modifier to Color: \"{}\"", *this)};
    }

} // acmacs::color::Modifier::operator Color

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
