#pragma once

// helper functions for accessing rjson::v3 values

#include "acmacs-base/color-modifier.hh"
#include "acmacs-base/rjson-v3.hh"
#include "acmacs-base/size-scale.hh"

// ----------------------------------------------------------------------

namespace rjson::v3
{
    template <typename Target> std::optional<Target> read_number(const rjson::v3::value& source)
    {
        return source.visit([]<typename Val>(const Val& value) -> std::optional<Target> {
            if constexpr (std::is_same_v<Val, rjson::v3::detail::number>)
                return value.template to<Target>();
            else if constexpr (std::is_same_v<Val, rjson::v3::detail::null>)
                return std::nullopt;
            else
                throw error{fmt::format("unrecognized: {}", value)};
        });

    } // read_number

    extern template std::optional<size_t  > read_number<size_t  >(const rjson::v3::value&);
    extern template std::optional<double  > read_number<double  >(const rjson::v3::value&);
    extern template std::optional<Pixels  > read_number<Pixels  >(const rjson::v3::value&);
    extern template std::optional<Scaled  > read_number<Scaled  >(const rjson::v3::value&);
    extern template std::optional<Rotation> read_number<Rotation>(const rjson::v3::value&);
    extern template std::optional<Aspect  > read_number<Aspect  >(const rjson::v3::value&);

    // ----------------------------------------------------------------------

    template <typename Target> Target read_number(const rjson::v3::value& source, Target&& dflt)
    {
        return source.visit([&dflt]<typename Val>(const Val& value) -> Target {
            if constexpr (std::is_same_v<Val, rjson::v3::detail::number>)
                return value.template to<Target>();
            else if constexpr (std::is_same_v<Val, rjson::v3::detail::null>)
                return std::forward<Target>(dflt);
            else
                throw error{fmt::format("unrecognized: {}", value)};
        });

    } // read_number

    extern template size_t   read_number<size_t>(const rjson::v3::value&, size_t&&);
    extern template double   read_number<double>(const rjson::v3::value&, double&&);
    extern template Pixels   read_number<Pixels>(const rjson::v3::value&, Pixels&&);
    extern template Scaled   read_number<Scaled>(const rjson::v3::value&, Scaled&&);
    extern template Rotation read_number<Rotation>(const rjson::v3::value&, Rotation&&);
    extern template Aspect   read_number<Aspect>(const rjson::v3::value&, Aspect&&);

    // ----------------------------------------------------------------------

    std::optional<acmacs::color::Modifier> read_color(const rjson::v3::value& source);
    acmacs::color::Modifier read_color_or_empty(const rjson::v3::value& source);
    std::optional<std::string_view> read_string(const rjson::v3::value& source);

} // namespace rjson::v3

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
