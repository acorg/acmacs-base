#pragma once

#include <optional>

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdocumentation-unknown-command"
#pragma GCC diagnostic ignored "-Wextra-semi-stmt" // fmt/format.h:1242
#pragma GCC diagnostic ignored "-Wundefined-func-template"
#pragma GCC diagnostic ignored "-Wsign-conversion" // fmt/format.h:2699
#pragma GCC diagnostic ignored "-Wdouble-promotion" // fmt/core.h:769
#pragma GCC diagnostic ignored "-Wsigned-enum-bitfield" // fmt/format.h
#pragma GCC diagnostic ignored "-Wshadow" // fmt/chrono.h
#pragma GCC diagnostic ignored "-Wshadow-field" // fmt/core.h
#pragma GCC diagnostic ignored "-Wundef" // fmt/core.h
#pragma GCC diagnostic ignored "-Wunused-template" // fmt/chrono.h
#endif

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/chrono.h>

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

namespace acmacs
{
    struct fmt_default_formatter
    {
    };

    struct fmt_float_formatter
    {
    };
}

// ----------------------------------------------------------------------

template <> struct fmt::formatter<acmacs::fmt_default_formatter>
{
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }
};

template <> struct fmt::formatter<acmacs::fmt_float_formatter>
{
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx)
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it == ':')
            ++it;
        const auto end = std::find(it, ctx.end(), '}');
        format_ = fmt::format("{{:{}}}", std::string(it, end));
        return end;
    }

    template <typename Val> std::string format_val(Val&& val)
    {
        return fmt::format(format_, std::forward<Val>(val));
    }

    template <typename Val, typename FormatContext> auto format_val(Val&& val, FormatContext& ctx)
    {
        return format_to(ctx.out(), format_, std::forward<Val>(val));
    }

  private:
    std::string format_;
};

// ----------------------------------------------------------------------

template <typename T> struct fmt::formatter<T, std::enable_if_t<std::is_base_of_v<std::exception, T>, char>> : fmt::formatter<const char*> {
    template <typename FormatCtx> auto format(const std::exception& err, FormatCtx& ctx) { return fmt::formatter<const char*>::format(err.what(), ctx); }
};

// template <> struct fmt::formatter<std::exception>
// {
//     template <typename ParseContext> constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }
//     template <typename FormatContext> auto format(const std::exception& err, FormatContext& ctx) { return format_to(ctx.out(), "{}", err.what()); }
// };

// template <> struct fmt::formatter<###> : fmt::formatter<acmacs::fmt_default_formatter> {
//     template <typename FormatCtx> auto format(const ###& value, FormatCtx& ctx)
//     {
//         format_to(ctx.out(), "{} {}", );
//         return format_to(ctx.out(), "{} {}", );
//         return ctx.out();
//     }
// };

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
