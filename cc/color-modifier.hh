#pragma once

#include <variant>

#include "acmacs-base/named-type.hh"
#include "acmacs-base/color.hh"

// ----------------------------------------------------------------------

namespace acmacs::color
{

    class Modifier
    {
      public:
        using hue = named_double_t<struct hue_tag>; // [-1..0) - to yellow-red, (0..1] - to magenta-red, -1 and 1 - red, 0 - no change
        using saturation = named_double_t<struct saturation_tag>; // [-1..0) - desaturate (pale), (0..1] saturate, -1 - white, 1 - full saturation, 0 - no change
        using brightness = named_double_t<struct brightness_tag>; // [-1..0)- darker, (0..1] - lighter, -1 - black, 1 - full bright, 0 - no change
        using transparency = named_double_t<struct transparency_tag>; // [-1..0) - more opaque, (0..1] - more transparent, -1 - full opaque, 1 - full transparent, 0 - no change
        using applicator_t = std::variant<Color, hue, saturation, brightness, transparency>;
        using applicators_t  = std::vector<applicator_t>;

        Modifier() = default;
        Modifier(Color color) : applicators_{color} {}
        Modifier(std::string_view source); // see ~/AD/share/doc/color.org

        constexpr const auto& applicators() const { return applicators_; }
        constexpr bool is_no_change() const { return applicators_.empty(); }

        operator Color() const;

      private:
        applicators_t applicators_; // no change if empty, applied in order stored
    };

    // inline bool operator==(const Modifier& lhs, const Modifier& rhs)
    // {
    //     return lhs.applicators() == rhs.applicators();
    // }

    inline auto operator==(const Modifier& lhs, const Modifier& rhs)
    {
        return lhs.applicators() == rhs.applicators();
    }

    inline auto operator<(const Modifier& lhs, const Modifier& rhs)
    {
        return lhs.applicators() < rhs.applicators();
    }

    // constexpr void set_transparency(Modifier& color, double transparency)
    // {
    //     std::visit(
    //         [transparency]<typename Value>(Value& value) {
    //             if constexpr (std::is_same_v<Color, Value>)
    //                 value.set_transparency(transparency);
    //             else
    //                 throw error{"acmacs::color::Modifier: set_transparency is not applicable to adjust color value"};
    //         },
    //         color);
    // }

    // constexpr void set_opacity(Modifier& color, double opacity) { set_transparency(color, 1.0 - opacity); }

    // constexpr Color without_transparency(const Modifier& color)
    // {
    //     return std::visit(
    //         []<typename Value>(const Value& value) -> Color {
    //             if constexpr (std::is_same_v<Color, Value>)
    //                 return value.without_transparency();
    //             else
    //                 throw error{"acmacs::color::Modifier: without_transparency is not applicable to adjust color value"};
    //         },
    //         color);
    // }

    // constexpr double opacity(const Modifier& color)
    // {
    //     return std::visit(
    //         []<typename Value>(const Value& value) -> double {
    //             if constexpr (std::is_same_v<Color, Value>)
    //                 return value.opacity();
    //             else
    //                 throw error{"acmacs::color::Modifier: opacity is not applicable to adjust color value"};
    //         },
    //         color);
    // }

    // constexpr double alpha(const Modifier& color)
    // {
    //     return std::visit(
    //         []<typename Value>(const Value& value) -> double {
    //             if constexpr (std::is_same_v<Color, Value>)
    //                 return value.alpha();
    //             else
    //                 throw error{"acmacs::color::Modifier: alpha() is not applicable to adjust color value"};
    //         },
    //         color);
    // }

    // constexpr bool operator==(const Modifier& lhs, Color rhs)
    // {
    //     return std::visit(
    //         [rhs]<typename Value>(const Value& value) -> bool {
    //             if constexpr (std::is_same_v<Color, Value>)
    //                 return value == rhs;
    //             else
    //                 throw error{"acmacs::color::Modifier: operator==() is not applicable to adjust color value"};
    //         },
    //         lhs);
    // }

    // constexpr bool operator!=(const Modifier& lhs, Color rhs) { return !operator==(lhs, rhs); }

    // constexpr const Color& get(const Modifier& color)
    // {
    //     return std::visit(
    //         []<typename Value>(const Value& value) -> const Color& {
    //             if constexpr (std::is_same_v<Color, Value>)
    //                 return value;
    //             else
    //                 throw error{"acmacs::color::Modifier: get() is not applicable to adjust color value"};
    //         },
    //         color);
    // }

    // constexpr bool is_no_change(const Modifier& color) noexcept
    // {
    //     return std::visit(
    //         []<typename Value>(const Value&) {
    //             if constexpr (std::is_same_v<no_change_, Value>)
    //                 return true;
    //             else
    //                 return false;
    //         },
    //         color);
    // }

} // namespace acmacs::color

// ----------------------------------------------------------------------

template <> struct fmt::formatter<acmacs::color::Modifier::hue> : fmt::formatter<acmacs::fmt_default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::color::Modifier::hue& hue, FormatContext& ctx) { return format_to(ctx.out(), ":h{:.4f}", *hue); }
};

template <> struct fmt::formatter<acmacs::color::Modifier::saturation> : fmt::formatter<acmacs::fmt_default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::color::Modifier::saturation& saturation, FormatContext& ctx) { return format_to(ctx.out(), ":s{:.4f}", *saturation); }
};

template <> struct fmt::formatter<acmacs::color::Modifier::brightness> : fmt::formatter<acmacs::fmt_default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::color::Modifier::brightness& brightness, FormatContext& ctx) { return format_to(ctx.out(), ":b{:.4f}", *brightness); }
};

template <> struct fmt::formatter<acmacs::color::Modifier::transparency> : fmt::formatter<acmacs::fmt_default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::color::Modifier::transparency& transparency, FormatContext& ctx) { return format_to(ctx.out(), ":t{:.4f}", *transparency); }
};

template <> struct fmt::formatter<acmacs::color::Modifier> : fmt::formatter<Color>
{
    template <typename FormatCtx> auto format(const acmacs::color::Modifier& modifier, FormatCtx& ctx)
    {
        for (const auto& app : modifier.applicators())
            std::visit(
                [&ctx, this]<typename Col>(const Col& value) {
                    if constexpr (std::is_same_v<Col, Color>)
                        return format_to(ctx.out(), fmt::format("{{:{}}}", format_code()), value);
                    else
                        return format_to(ctx.out(), "{}", value);
                },
                app);
        return ctx.out();
    }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
