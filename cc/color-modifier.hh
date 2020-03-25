#pragma once

#include <variant>

#include "acmacs-base/named-type.hh"
#include "acmacs-base/color.hh"

// ----------------------------------------------------------------------

namespace acmacs::color
{
    enum no_change_ { no_change };
    using adjust_saturation = named_double_t<struct adjust_saturation_tag>;
    using adjust_brightness = named_double_t<struct adjust_brightness_tag>;
    using adjust_transparency = named_double_t<struct adjust_transparency_tag>;

    using Modifier = std::variant<Color, no_change_, adjust_saturation, adjust_brightness, adjust_transparency>;

    constexpr void set_transparency(Modifier& color, double transparency)
    {
        std::visit(
            [transparency]<typename Value>(Value& value) {
                if constexpr (std::is_same_v<Color, Value>)
                    value.set_transparency(transparency);
                else
                    throw error{"acmacs::color::Modifier: set_transparency is not applicable to adjust color value"};
            },
            color);
    }

    constexpr void set_opacity(Modifier& color, double opacity) { set_transparency(color, 1.0 - opacity); }

    constexpr Color without_transparency(const Modifier& color)
    {
        return std::visit(
            []<typename Value>(const Value& value) -> Color {
                if constexpr (std::is_same_v<Color, Value>)
                    return value.without_transparency();
                else
                    throw error{"acmacs::color::Modifier: without_transparency is not applicable to adjust color value"};
            },
            color);
    }

    constexpr double opacity(const Modifier& color)
    {
        return std::visit(
            []<typename Value>(const Value& value) -> double {
                if constexpr (std::is_same_v<Color, Value>)
                    return value.opacity();
                else
                    throw error{"acmacs::color::Modifier: opacity is not applicable to adjust color value"};
            },
            color);
    }

    constexpr double alpha(const Modifier& color)
    {
        return std::visit(
            []<typename Value>(const Value& value) -> double {
                if constexpr (std::is_same_v<Color, Value>)
                    return value.alpha();
                else
                    throw error{"acmacs::color::Modifier: alpha() is not applicable to adjust color value"};
            },
            color);
    }

    constexpr bool operator==(const Modifier& lhs, Color rhs)
    {
        return std::visit(
            [rhs]<typename Value>(const Value& value) -> bool {
                if constexpr (std::is_same_v<Color, Value>)
                    return value == rhs;
                else
                    throw error{"acmacs::color::Modifier: operator==() is not applicable to adjust color value"};
            },
            lhs);
    }

    constexpr bool operator!=(const Modifier& lhs, Color rhs) { return !operator==(lhs, rhs); }

    constexpr const Color& get(const Modifier& color)
    {
        return std::visit(
            []<typename Value>(const Value& value) -> const Color& {
                if constexpr (std::is_same_v<Color, Value>)
                    return value;
                else
                    throw error{"acmacs::color::Modifier: get() is not applicable to adjust color value"};
            },
            color);
    }

    constexpr bool is_no_change(const Modifier& color) noexcept
    {
        return std::visit(
            []<typename Value>(const Value&) {
                if constexpr (std::is_same_v<no_change_, Value>)
                    return true;
                else
                    return false;
            },
            color);
    }

} // namespace acmacs::color

// ----------------------------------------------------------------------

template <> struct fmt::formatter<acmacs::color::Modifier> : fmt::formatter<Color> {
    template <typename FormatCtx> auto format(const acmacs::color::Modifier& color, FormatCtx& ctx)
    {
        return std::visit(
            [&ctx, this]<typename Value>(const Value& value) {
                if constexpr (std::is_same_v<Color, Value>)
                    return format_to(ctx.out(), fmt::format("{{:{}}}", format_code()), value);
                else if constexpr (std::is_same_v<acmacs::color::no_change_, Value>)
                    return format_to(ctx.out(), "[no-change]");
                else if constexpr (std::is_same_v<acmacs::color::adjust_saturation, Value>)
                    return format_to(ctx.out(), "[adjust_saturation:{}]", value);
                else if constexpr (std::is_same_v<acmacs::color::adjust_brightness, Value>)
                    return format_to(ctx.out(), "[adjust_brightness:{}]", value);
                else if constexpr (std::is_same_v<acmacs::color::adjust_transparency, Value>)
                    return format_to(ctx.out(), "[adjust_transparency:{}]", value);
                else
                    static_assert(std::is_same_v<Color, Value>);
            },
            color);
    }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
