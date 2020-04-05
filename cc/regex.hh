#pragma once

#include <regex>

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

// specialization below follows fmt lib description, but it does not work due to ambiguity with
// template <typename RangeT, typename Char> struct formatter<RangeT, Char, enable_if_t<fmt::is_range<RangeT, Char>::value>>
//
// template <typename Match>  struct fmt::formatter<Match, std::enable_if_t<std::is_same_v<Match, std::smatch> || std::is_same_v<Match, std::cmatch>, char>> : fmt::formatter<acmacs::fmt_default_formatter>

// ----------------------------------------------------------------------

namespace acmacs
{
    template <typename Match> struct fmt_regex_match_formatter {};
}

template <typename Match> struct fmt::formatter<acmacs::fmt_regex_match_formatter<Match>> : fmt::formatter<acmacs::fmt_default_formatter>
{
    template <typename FormatCtx> auto format(const Match& mr, FormatCtx& ctx) {
        format_to(ctx.out(), "\"{}\" -> ({})[", mr.str(0), mr.size());
        for (size_t nr = 1; nr <= mr.size(); ++nr)
            format_to(ctx.out(), " {}:\"{}\"", nr, mr.str(nr));
        format_to(ctx.out(), " ]");
        return ctx.out();
    }
};

template <> struct fmt::formatter<std::smatch> : fmt::formatter<acmacs::fmt_regex_match_formatter<std::smatch>>
{
};

template <> struct fmt::formatter<std::cmatch> : fmt::formatter<acmacs::fmt_regex_match_formatter<std::cmatch>>
{
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
