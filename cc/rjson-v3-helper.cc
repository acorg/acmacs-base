#include "acmacs-base/rjson-v3-helper.hh"

// ----------------------------------------------------------------------

template std::optional<size_t  > rjson::v3::read_number<size_t  >(const rjson::v3::value&);
template std::optional<double  > rjson::v3::read_number<double  >(const rjson::v3::value&);
template std::optional<Pixels  > rjson::v3::read_number<Pixels  >(const rjson::v3::value&);
template std::optional<Scaled  > rjson::v3::read_number<Scaled  >(const rjson::v3::value&);
template std::optional<Rotation> rjson::v3::read_number<Rotation>(const rjson::v3::value&);
template std::optional<Aspect  > rjson::v3::read_number<Aspect  >(const rjson::v3::value&);

template size_t   rjson::v3::read_number<size_t>(const rjson::v3::value&, size_t&&);
template double   rjson::v3::read_number<double>(const rjson::v3::value&, double&&);
template Pixels   rjson::v3::read_number<Pixels>(const rjson::v3::value&, Pixels&&);
template Scaled   rjson::v3::read_number<Scaled>(const rjson::v3::value&, Scaled&&);
template Rotation rjson::v3::read_number<Rotation>(const rjson::v3::value&, Rotation&&);
template Aspect   rjson::v3::read_number<Aspect>(const rjson::v3::value&, Aspect&&);

// ----------------------------------------------------------------------

std::optional<acmacs::color::Modifier> rjson::v3::read_color(const rjson::v3::value& source)
{
    return source.visit([]<typename Val>(const Val& value) -> std::optional<acmacs::color::Modifier> {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::string>)
            return acmacs::color::Modifier{value.template to<std::string_view>()};
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::null>)
            return std::nullopt;
        else
            throw error{fmt::format("unrecognized: {} (expected color)", value)};
    });

} // rjson::v3::read_color

// ----------------------------------------------------------------------

acmacs::color::Modifier rjson::v3::read_color_or_empty(const rjson::v3::value& source)
{
    if (auto color = read_color(source); color.has_value())
        return *color;
    else
        return {};

} // rjson::v3::read_color_or_empty

// ----------------------------------------------------------------------

std::optional<std::string_view> rjson::v3::read_string(const rjson::v3::value& source)
{
    return source.visit([]<typename Val>(const Val& value) -> std::optional<std::string_view> {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::string>)
            return value.template to<std::string_view>();
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::null>)
            return std::nullopt;
        else
            throw error{fmt::format("unrecognized: {} (expected string)", value)};
    });

} // rjson::v3::read_string

// ----------------------------------------------------------------------

std::string_view rjson::v3::read_string(const rjson::v3::value& source, std::string_view dflt)
{
    if (auto str = read_string(source); str.has_value())
        return *str;
    else
        return dflt;

} // rjson::v3::read_string

// ----------------------------------------------------------------------

bool rjson::v3::read_bool(const rjson::v3::value& source, bool dflt)
{
    return source.visit([dflt]<typename Val>(const Val& value) -> bool {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::null>)
            return dflt;
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::boolean>)
            return value.template to<bool>();
        else
            throw error{fmt::format("unrecognized {} (expected boolean)", value)};
    });

} // rjson::v3::read_bool

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
