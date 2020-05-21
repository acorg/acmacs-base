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
        // $ACMACSD_ROOT/share/doc/color.org
        using hue_set = named_double_t<struct hue_set_tag>; // 0.0 - 360.0
        using hue_adjust = named_double_t<struct hue_adjust_tag>; // -1.0 - +1.0
        using saturation_set = named_double_t<struct saturation_set_tag>;
        using saturation_adjust = named_double_t<struct saturation_adjust_tag>;
        using brightness_set = named_double_t<struct brightness_set_tag>;
        using brightness_adjust = named_double_t<struct brightness_adjust_tag>;
        using transparency_set = named_double_t<struct transparency_set_tag>;
        using transparency_adjust = named_double_t<struct transparency_adjust_tag>;

        using applicator_t = std::variant<Color, hue_set, hue_adjust, saturation_set, saturation_adjust, brightness_set, brightness_adjust, transparency_set, transparency_adjust>;
        using applicators_t  = std::vector<applicator_t>;

        Modifier() = default;
        template <typename Appl, typename = std::enable_if_t<std::is_constructible_v<applicator_t, Appl>>> explicit Modifier(const Appl& app) : applicators_{applicator_t{app}} {}
        Modifier(std::string_view source); // see ~/AD/share/doc/color.org
        Modifier& operator=(Color color) { return operator=(Modifier{color}); }

        Modifier& add(const Modifier& rhs);

        constexpr const auto& applicators() const { return applicators_; }
        /*constexpr*/ bool is_no_change() const { return applicators_.empty(); }

        operator Color() const;

      private:
        applicators_t applicators_; // no change if empty, applied in order stored

        applicators_t::iterator find_last_color() noexcept;
        applicators_t::const_iterator find_last_color() const noexcept;
    };

    inline bool operator==(const Modifier& lhs, const Modifier& rhs)
    {
        return lhs.applicators() == rhs.applicators();
    }

    inline auto operator!=(const Modifier& lhs, const Modifier& rhs)
    {
        return !operator==(lhs, rhs);
    }

    inline auto operator<(const Modifier& lhs, const Modifier& rhs)
    {
        return lhs.applicators() < rhs.applicators();
    }

    Color& modify(Color& target, const Modifier& modifier);

} // namespace acmacs::color

// ----------------------------------------------------------------------

template <> struct fmt::formatter<acmacs::color::Modifier::hue_set> : fmt::formatter<acmacs::fmt_helper::default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::color::Modifier::hue_set& hue, FormatContext& ctx)
    {
        if (*hue < 1.0)
            return format_to(ctx.out(), ":h={:+.4f}", *hue);
        else
            return format_to(ctx.out(), ":h={:.0f}", *hue);
    }
};

template <> struct fmt::formatter<acmacs::color::Modifier::hue_adjust> : fmt::formatter<acmacs::fmt_helper::default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::color::Modifier::hue_adjust& hue, FormatContext& ctx) { return format_to(ctx.out(), ":h{:+.4f}", *hue); }
};

template <> struct fmt::formatter<acmacs::color::Modifier::saturation_set> : fmt::formatter<acmacs::fmt_helper::default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::color::Modifier::saturation_set& saturation, FormatContext& ctx) { return format_to(ctx.out(), ":s={:.4f}", *saturation); }
};

template <> struct fmt::formatter<acmacs::color::Modifier::saturation_adjust> : fmt::formatter<acmacs::fmt_helper::default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::color::Modifier::saturation_adjust& saturation, FormatContext& ctx) { return format_to(ctx.out(), ":s{:+.4f}", *saturation); }
};

template <> struct fmt::formatter<acmacs::color::Modifier::brightness_set> : fmt::formatter<acmacs::fmt_helper::default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::color::Modifier::brightness_set& brightness, FormatContext& ctx) { return format_to(ctx.out(), ":b={:.4f}", *brightness); }
};

template <> struct fmt::formatter<acmacs::color::Modifier::brightness_adjust> : fmt::formatter<acmacs::fmt_helper::default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::color::Modifier::brightness_adjust& brightness, FormatContext& ctx) { return format_to(ctx.out(), ":b{:+.4f}", *brightness); }
};

template <> struct fmt::formatter<acmacs::color::Modifier::transparency_set> : fmt::formatter<acmacs::fmt_helper::default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::color::Modifier::transparency_set& transparency, FormatContext& ctx) { return format_to(ctx.out(), ":t={:.4f}", *transparency); }
};

template <> struct fmt::formatter<acmacs::color::Modifier::transparency_adjust> : fmt::formatter<acmacs::fmt_helper::default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::color::Modifier::transparency_adjust& transparency, FormatContext& ctx) { return format_to(ctx.out(), ":t{:+.4f}", *transparency); }
};

template <> struct fmt::formatter<acmacs::color::Modifier> : fmt::formatter<Color>
{
    template <typename FormatCtx> auto format(const acmacs::color::Modifier& modifier, FormatCtx& ctx)
    {
        for (const auto& app : modifier.applicators()) {
            std::visit(
                [&ctx, this]<typename Col>(const Col& value) {
                    if constexpr (std::is_same_v<Col, Color>)
                        return format_to(ctx.out(), fmt::format("{{:{}}}", format_code()), value);
                    else
                        return format_to(ctx.out(), "{}", value);
                },
                app);
        }
        return ctx.out();
    }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
