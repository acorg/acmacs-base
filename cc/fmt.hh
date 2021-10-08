#pragma once

#include <optional>
#include <tuple>
#include <vector>
#include <utility>

#pragma GCC diagnostic push
#ifdef __clang__
// 8.0.1, clang 13
#pragma GCC diagnostic ignored "-Wreserved-identifier" // identifier '_a' is reserved because it starts with '_' at global scope (bug in clang13 ?)
#pragma GCC diagnostic ignored "-Wdocumentation-unknown-command"
#pragma GCC diagnostic ignored "-Wsigned-enum-bitfield" // fmt/format.h
#pragma GCC diagnostic ignored "-Wmissing-noreturn" // fmt/core.h
#pragma GCC diagnostic ignored "-Wundefined-func-template" // fmt/chrono.h:1182

// #pragma GCC diagnostic ignored "-Wextra-semi-stmt" // fmt/format.h:1242
// #pragma GCC diagnostic ignored "-Wsign-conversion" // fmt/format.h:2699
// #pragma GCC diagnostic ignored "-Wdouble-promotion" // fmt/core.h:769
// #pragma GCC diagnostic ignored "-Wshadow" // fmt/chrono.h
// #pragma GCC diagnostic ignored "-Wshadow-field" // fmt/core.h
// #pragma GCC diagnostic ignored "-Wundef" // fmt/core.h
// #pragma GCC diagnostic ignored "-Wunused-template" // fmt/chrono.h
// #pragma GCC diagnostic ignored "-Wnon-virtual-dtor" // fmt/core.h


// clang 11
// #pragma GCC diagnostic ignored "-Wsuggest-override"
// #pragma GCC diagnostic ignored "-Wsuggest-destructor-override"

// #pragma GCC diagnostic ignored ""
// #pragma GCC diagnostic ignored ""
// #pragma GCC diagnostic ignored ""
#endif

#ifdef __GNUG__
#pragma GCC diagnostic ignored "-Wdeprecated" // fmt/format.h: implicit capture of ‘this’ via ‘[=]’ is deprecated in C++20
#endif

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/chrono.h>

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

namespace acmacs::fmt_helper
{
    struct default_formatter
    {
    };

    struct float_formatter
    {
    };
}

// ----------------------------------------------------------------------

template <> struct fmt::formatter<acmacs::fmt_helper::default_formatter>
{
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }
};

template <> struct fmt::formatter<acmacs::fmt_helper::float_formatter>
{
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx)
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it == ':')
            ++it;
        const auto end = std::find(it, ctx.end(), '}');
        format_ = fmt::format("{{:{}}}", std::string_view(it, static_cast<size_t>(end - it)));
        return end;
    }

    template <typename Val> std::string format_val(Val&& val) const
    {
        return fmt::format(fmt::runtime(format_), std::forward<Val>(val));
    }

    template <typename Val, typename FormatContext> auto format_val(Val&& val, FormatContext& ctx) const
    {
        return format_to(ctx.out(), fmt::runtime(format_), std::forward<Val>(val));
    }

  private:
    std::string format_;
};

// ----------------------------------------------------------------------

template <typename T> struct fmt::formatter<T, std::enable_if_t<std::is_base_of_v<std::exception, T>, char>> : fmt::formatter<const char*> {
    template <typename FormatCtx> auto format(const std::exception& err, FormatCtx& ctx) const { return fmt::formatter<const char*>::format(err.what(), ctx); }
};

// template <> struct fmt::formatter<std::exception>
// {
//     template <typename ParseContext> constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }
//     template <typename FormatContext> auto format(const std::exception& err, FormatContext& ctx) { return format_to(ctx.out(), "{}", err.what()); }
// };

// template <> struct fmt::formatter<###> : fmt::formatter<acmacs::fmt_helper::default_formatter> {
//     template <typename FormatCtx> auto format(const ###& value, FormatCtx& ctx)
//     {
//         format_to(ctx.out(), "{} {}", );
//         return format_to(ctx.out(), "{} {}", );
//         return ctx.out();
//     }
// };

// ----------------------------------------------------------------------

namespace fmt
{
    template <typename T> std::string format(const std::vector<T>& collection, std::string_view entry_format, std::string_view entry_separator = "\n  ")
    {
        return fmt::format(entry_format, fmt::join(collection, entry_separator));
    }

} // namespace fmt

// ----------------------------------------------------------------------
// memory_buffer unexpected problem
// in 8.0 format_to(memory_buffer,...) is deprecated without clearly stating in docs
// https://www.gitmemory.com/issue/fmtlib/fmt/2420/877703767
// ----------------------------------------------------------------------

namespace fmt
{
    template <typename BUF, typename... T> inline void format_to_mb(BUF& buf, format_string<T...> fmt, T&&... args)
    {
        detail::vformat_to(buf, string_view(fmt), fmt::make_format_args(args...));
    }
}

// ----------------------------------------------------------------------
// substitute
// ----------------------------------------------------------------------

namespace fmt
{
    enum class if_no_substitution_found { leave_as_is, empty };

    std::vector<std::pair<std::string_view, std::string_view>> split_for_formatting(std::string_view source); // pair{key, format}: {"key", "{key:03d}"} {"", "between-format"}

    template <typename FormatMatched, typename FormatNoPattern, typename... Args>
    void substitute_to(FormatMatched&& format_matched, FormatNoPattern&& format_no_pattern, std::string_view pattern, if_no_substitution_found insf, Args&&... args)
    {
        const auto match_and_format = [&format_matched](std::string_view look_for, std::string_view pattern_arg, const auto& en) {
            if (look_for == std::get<0>(en)) {
                format_matched(pattern_arg, en);
                return true;
            }
            else
                return false;
        };

        for (const auto& [key, pattern_arg] : split_for_formatting(pattern)) {
            if (!key.empty()) {
                if (!(match_and_format(key, pattern_arg, args) || ...)) {
                    // not matched
                    switch (insf) {
                        case if_no_substitution_found::leave_as_is:
                            format_no_pattern(pattern_arg);
                            break;
                        case if_no_substitution_found::empty:
                            break;
                    }
                }
            }
            else
                format_no_pattern(pattern_arg);
        }
    }

    // substitute_to args:
    // std::pair{"name", value}                                     -- {name}, {name:3d}
    // std::pair{"name", []() -> decltype(value) { return value; }} -- {name}, {name:3d}
    // std::tuple{"name1", val1, "name2", val2}                     -- {name1:{name2}d}
    template <typename... Args> void substitute_to(memory_buffer& output, std::string_view pattern, if_no_substitution_found insf, Args&&... args)
    {
        const auto format_matched = [&output](std::string_view pattern_arg, const auto& key_value) {
            static_assert(std::is_same_v<std::decay_t<decltype(std::get<0>(key_value))>, const char*>);
            if constexpr (std::tuple_size<std::decay_t<decltype(key_value)>>::value == 2) {
                if constexpr (std::is_invocable_v<decltype(std::get<1>(key_value))>)
                    format_to_mb(output, fmt::runtime(pattern_arg), arg(std::get<0>(key_value), std::invoke(std::get<1>(key_value))));
                else
                    format_to_mb(output, fmt::runtime(pattern_arg), arg(std::get<0>(key_value), std::get<1>(key_value)));
            }
            else if constexpr (std::tuple_size<std::decay_t<decltype(key_value)>>::value == 4) {
                format_to_mb(output, fmt::runtime(pattern_arg), arg(std::get<0>(key_value), std::get<1>(key_value)), arg(std::get<2>(key_value), std::get<3>(key_value)));
            }
            else
                static_assert(
                    std::tuple_size<std::decay_t<decltype(key_value)>>::value == 0,
                    "fmt::substitute arg can be used in the following forms: std::pair<const char*, value>, std::pair<const char*, lambda>, std::tuple<const char*, value, const char*, value>");
        };
        const auto format_no_pattern = [&output](std::string_view no_pattern) { output.append(no_pattern); };
        substitute_to(format_matched, format_no_pattern, pattern, insf, std::forward<Args>(args)...);
    }

    // see acmacs-chart-2/cc/name-format.cc for usage example

    template <typename... Args> std::string substitute(std::string_view pattern, if_no_substitution_found insf, Args&&... args)
    {
        memory_buffer output;
        substitute_to(output, pattern, insf, std::forward<Args>(args)...);
        return to_string(output);
    }

    template <typename... Args> std::string substitute(std::string_view pattern, Args&&... args) { return substitute(pattern, if_no_substitution_found::leave_as_is, std::forward<Args>(args)...); }

} // namespace fmt

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
