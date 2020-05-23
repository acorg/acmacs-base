#pragma once

// helper functions for accessing rjson::v3 values

#include "acmacs-base/rjson-v3.hh"
#include "acmacs-base/color-modifier.hh"

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

    // ----------------------------------------------------------------------

    inline std::optional<acmacs::color::Modifier> read_color(const rjson::v3::value& source)
    {
        return source.visit([]<typename Val>(const Val& value) -> std::optional<acmacs::color::Modifier> {
            if constexpr (std::is_same_v<Val, rjson::v3::detail::string>)
                return acmacs::color::Modifier{value.template to<std::string_view>()};
            else if constexpr (std::is_same_v<Val, rjson::v3::detail::null>)
                return std::nullopt;
            else
                throw error{fmt::format("unrecognized: {}", value)};
        });

    } // read_color

    // ----------------------------------------------------------------------

    inline acmacs::color::Modifier read_color_or_empty(const rjson::v3::value& source)
    {
        return source.visit([]<typename Val>(const Val& value) -> acmacs::color::Modifier {
            if constexpr (std::is_same_v<Val, rjson::v3::detail::string>)
                return value.template to<std::string_view>();
            else if constexpr (std::is_same_v<Val, rjson::v3::detail::null>)
                return {};
            else
                throw error{fmt::format("unrecognized: {}", value)};
        });

    } // read_color_or_empty

    // ----------------------------------------------------------------------

    inline std::optional<std::string_view> read_from_string(const rjson::v3::value& source)
    {
        return source.visit([]<typename Val>(const Val& value) -> std::optional<std::string_view> {
            if constexpr (std::is_same_v<Val, rjson::v3::detail::string>)
                return value.template to<std::string_view>();
            else if constexpr (std::is_same_v<Val, rjson::v3::detail::null>)
                return std::nullopt;
            else
                throw error{fmt::format("unrecognized: {}", value)};
        });

    } // read_from_string

} // namespace rjson::v3

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
