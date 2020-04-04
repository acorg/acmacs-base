#pragma once

#include <regex>

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

// template <typename T> struct fmt::formatter<std::match_results<T>> : fmt::formatter<acmacs::fmt_default_formatter> {
//     template <typename FormatCtx> auto format(const std::match_results<T>& mr, FormatCtx& ctx) {
template <> struct fmt::formatter<std::smatch> : fmt::formatter<acmacs::fmt_default_formatter> {
    template <typename FormatCtx> auto format(const std::smatch& mr, FormatCtx& ctx) {
        format_to(ctx.out(), "\"{}\" -> ({})[", mr.str(0), mr.size());
        for (size_t nr = 1; nr <= mr.size(); ++nr)
            format_to(ctx.out(), " {}:\"{}\"", nr, mr.str(nr));
        format_to(ctx.out(), " ]");
        return ctx.out();
    }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
