#pragma once

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
#pragma GCC diagnostic ignored "-Wunused-template" // fmt/chrono.h
#endif

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/chrono.h>

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

namespace acmacs
{
    template <typename ParseContext> inline std::string fmt_extract_format_float(const ParseContext& ctx, std::string_view a_default = {".4f"})
    {
        if (const auto end_of_fomat = std::find(ctx.begin(), ctx.end(), '}'); end_of_fomat != ctx.end())
            return std::string(*ctx.begin() == ':' ? std::next(ctx.begin()) : ctx.begin(), end_of_fomat);
        else
            return std::string{a_default};
    }

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
        format_float = acmacs::fmt_extract_format_float(ctx, ".4f");
        return ctx.begin();
    }

    std::string format_float;
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

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
