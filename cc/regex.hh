#pragma once

#include <regex>

#include "acmacs-base/fmt.hh"

// ======================================================================
// Support for multiple regex and replacements, see acmacs-virus/cc/reassortant.cc
// ======================================================================

namespace acmacs::regex
{
    struct look_replace_t
    {
        const std::regex look_for;
        const char* replace_fmt; // syntax: http://ecma-international.org/ecma-262/5.1/#sec-15.5.4.11
    };

    // returns empty string if no matches found
    template <typename Container> std::string scan_replace(std::string_view source, const Container& scan_data)
    {
        for (const auto& entry : scan_data) {
            if (std::cmatch match; std::regex_search(std::begin(source), std::end(source), match, entry.look_for))
                return match.format(entry.replace_fmt);
        }
        return {};
    }

    // ----------------------------------------------------------------------

    struct look_replace2_t
    {
        const std::regex look_for;
        const char* replace_fmt1;
        const char* replace_fmt2;
    };

    // returns pair of empty strings if no matches found
    template <typename Container> std::pair<std::string, std::string> scan_replace2(std::string_view source, const Container& scan_data)
    {
        for (const auto& entry : scan_data) {
            if (std::cmatch match; std::regex_search(std::begin(source), std::end(source), match, entry.look_for))
                return {match.format(entry.replace_fmt1), match.format(entry.replace_fmt2)};
        }
        return {};
    }

}

// ======================================================================
// fmt support for std::smatch and std::cmatch
// ======================================================================

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
