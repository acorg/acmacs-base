#pragma once

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    enum class debug { no, yes };
    enum class verbose { no, yes };

    template <typename Int> constexpr auto number_of_decimal_digits(Int max_value) { return static_cast<int>(std::log10(max_value)) + 1; }

    // constexpr inline debug debug_from(bool verb) { return verb ? debug::yes : debug::no; }
    constexpr inline verbose verbose_from(bool verb) { return verb ? verbose::yes : verbose::no; }

    // ----------------------------------------------------------------------

    namespace log::inline v1
    {
        // https://clang.llvm.org/docs/LanguageExtensions.html#source-location-builtins
        struct source_location
        {
            const char* file{__builtin_FILE()};
            int line{__builtin_LINE()};
            const char* function{__builtin_FUNCTION()};
        };

        struct log_key_t
        {
            explicit log_key_t(std::string_view kk) : key{kk} {}
            explicit log_key_t(const char* kk) : key{kk} {}
            bool operator==(const log_key_t& rhs) const { return key == rhs.key; }
            bool operator==(std::string_view rhs) const { return key == rhs; }
            operator std::string_view() const { return key; }
            operator const std::string&() const { return key; }
            std::string key;
        };

        extern const log_key_t all, timer, settings, vaccines;

        namespace detail
        {
            using enabled_t = std::vector<log_key_t>;
            extern enabled_t enabled;
            extern size_t indent;
            constexpr size_t indent_size{4};
            extern bool print_debug_messages; // to disable by acmacs.r

        } // namespace detail

        inline bool is_enabled(log_key_t section)
        {
            return !detail::enabled.empty() && (detail::enabled.front() == all || std::find(std::begin(detail::enabled), std::end(detail::enabled), section) != std::end(detail::enabled));
        }

        template <typename MesssageGetter> void message(log_key_t section, MesssageGetter get_message)
        {
            if (is_enabled(section))
                fmt::print(stderr, ">>>> [{}]: {:{}s}{}\n", section.key, "", detail::indent, get_message());
        }

        void enable(std::string_view names);
        inline void enable(log_key_t name) { return enable(name.key); }
        void enable(const std::vector<std::string_view>& names);
        std::string report_enabled();

        struct indent
        {
            indent() { detail::indent += detail::indent_size; }
            ~indent() { detail::indent -= detail::indent_size; }
        };

        // ----------------------------------------------------------------------

        namespace prefix
        {
            constexpr const std::string_view error{"> ERROR"};
            constexpr const std::string_view warning{">> WARNING"};
            constexpr const std::string_view info{">>>"};
            constexpr const std::string_view debug{">>>>"};

        } // namespace prefix

        // ----------------------------------------------------------------------

        template <typename Fmt, typename... Ts> inline std::string format(const source_location& sl, Fmt format, Ts&&... ts)
        {
            try {
                return fmt::format(format, std::forward<Ts>(ts)...);
            }
            catch (fmt::format_error& err) {
                fmt::print(stderr, "> fmt::format_error ({}) format: \"{}\" {}", err, format, sl);
                throw;
            }
        }

        template <typename Fmt, typename... Ts> inline void print(const source_location& sl, bool do_print, std::string_view prefix, Fmt format, Ts&&... ts)
        {
            if (do_print && detail::print_debug_messages)
                fmt::print(stderr, "{} {} {}\n", prefix, acmacs::log::format(sl, format, std::forward<Ts>(ts)...), sl);
        }

        template <typename Fmt, typename... Ts> inline void ad_assert(bool condition, const source_location& sl, Fmt format, Ts&&... ts)
        {
            if (!condition) {
                print(sl, true, "> ASSERTION FAILED", format, std::forward<Ts>(ts)...);
                std::abort();
            }
        }

        // ----------------------------------------------------------------------

        inline void do_not_print_debug_messages() { detail::print_debug_messages = false; }

        // ----------------------------------------------------------------------

        // inline void debug_print(bool do_print, std::string_view prefix, std::string_view message, const char* filename, int line_no, [[maybe_unused]] const char* function)
        // {
        //     if (do_print && detail::print_debug_messages) {
        //         // fmt::print(stderr, "{} {} @@ {}:{} {}\n", prefix, message, filename, line_no, function);
        //         fmt::print(stderr, "{} {} @@ {}:{}\n", prefix, message, filename, line_no);
        //     }
        // }

        // inline void debug_print(bool do_print, std::string_view prefix, std::string_view message, const source_location& sl)
        // {
        //     if (do_print && detail::print_debug_messages)
        //         fmt::print(stderr, "{} {} {}\n", prefix, message, sl);
        // }

        // inline void ad_assert(bool condition, std::string_view message, const char* filename, int line_no, const char* function)
        // {
        //     if (!(condition)) {
        //         debug_print(true, "> ASSERTION FAILED", message, filename, line_no, function);
        //         std::abort();
        //     }
        // }

        // template <typename Fmt, typename... Ts> inline std::string try_format(const source_location& sl, Fmt format, Ts&&... ts)
        // {
        //     try {
        //         return fmt::format(format, std::forward<Ts>(ts)...);
        //     }
        //     catch (fmt::format_error& err) {
        //         acmacs::log::debug_print(true, ">", fmt::format("fmt::format_error ({}) format: \"{}\"", err, format), sl);
        //         throw;
        //     }
        // }

    } // namespace log::inline v1

} // namespace acmacs

// ----------------------------------------------------------------------

template <> struct fmt::formatter<acmacs::log::source_location> : fmt::formatter<acmacs::fmt_helper::default_formatter> {
    template <typename FormatCtx> auto format(const acmacs::log::source_location& sl, FormatCtx& ctx)
    {
        return format_to(ctx.out(), "@@ {}:{}", sl.file, sl.line);
    }
};

// ======================================================================

// https://www.cppstories.com/2021/non-terminal-variadic-args/

template <typename Fmt, typename... Ts> struct AD_ERROR
{
    AD_ERROR(Fmt format, Ts&&... ts, const acmacs::log::source_location& sl = acmacs::log::source_location{})
    {
        acmacs::log::print(sl, true, acmacs::log::prefix::error, format, std::forward<Ts>(ts)...);
    }
};

template <typename Fmt, typename... Ts> AD_ERROR(Fmt, Ts&&...) -> AD_ERROR<Fmt, Ts...>;

// ----------------------------------------------------------------------

template <typename Fmt, typename... Ts> struct AD_WARNING
{
    AD_WARNING(Fmt format, Ts&&... ts, const acmacs::log::source_location& sl = acmacs::log::source_location{})
    {
        acmacs::log::print(sl, true, acmacs::log::prefix::warning, format, std::forward<Ts>(ts)...);
    }
    AD_WARNING(bool do_print, Fmt format, Ts&&... ts, const acmacs::log::source_location& sl = acmacs::log::source_location{})
    {
        acmacs::log::print(sl, do_print, acmacs::log::prefix::warning, format, std::forward<Ts>(ts)...);
    }
};

template <typename Fmt, typename... Ts> AD_WARNING(Fmt, Ts&&...) -> AD_WARNING<Fmt, Ts...>;
template <typename Fmt, typename... Ts> AD_WARNING(bool, Fmt, Ts&&...) -> AD_WARNING<Fmt, Ts...>;

// ----------------------------------------------------------------------

template <typename Fmt, typename... Ts> struct AD_INFO
{
    AD_INFO(Fmt format, Ts&&... ts, const acmacs::log::source_location& sl = acmacs::log::source_location{})
    {
        acmacs::log::print(sl, true, acmacs::log::prefix::info, format, std::forward<Ts>(ts)...);
    }
    AD_INFO(bool do_print, Fmt format, Ts&&... ts, const acmacs::log::source_location& sl = acmacs::log::source_location{})
    {
        acmacs::log::print(sl, do_print, acmacs::log::prefix::info, format, std::forward<Ts>(ts)...);
    }
};

template <typename Fmt, typename... Ts> AD_INFO(Fmt, Ts&&...) -> AD_INFO<Fmt, Ts...>;
template <typename Fmt, typename... Ts> AD_INFO(bool, Fmt, Ts&&...) -> AD_INFO<Fmt, Ts...>;

// ----------------------------------------------------------------------

template <typename Fmt, typename... Ts> struct AD_DEBUG
{
    AD_DEBUG(Fmt format, Ts&&... ts, const acmacs::log::source_location& sl = acmacs::log::source_location{})
    {
        acmacs::log::print(sl, true, acmacs::log::prefix::debug, format, std::forward<Ts>(ts)...);
    }
    AD_DEBUG(bool do_print, Fmt format, Ts&&... ts, const acmacs::log::source_location& sl = acmacs::log::source_location{})
    {
        acmacs::log::print(sl, do_print, acmacs::log::prefix::debug, format, std::forward<Ts>(ts)...);
    }
    AD_DEBUG(acmacs::debug dbg, Fmt format, Ts&&... ts, const acmacs::log::source_location& sl = acmacs::log::source_location{})
    {
        acmacs::log::print(sl, dbg == acmacs::debug::yes, acmacs::log::prefix::debug, format, std::forward<Ts>(ts)...);
    }
};

template <typename Fmt, typename... Ts> AD_DEBUG(Fmt, Ts&&...) -> AD_DEBUG<Fmt, Ts...>;
template <typename Fmt, typename... Ts> AD_DEBUG(bool, Fmt, Ts&&...) -> AD_DEBUG<Fmt, Ts...>;
template <typename Fmt, typename... Ts> AD_DEBUG(acmacs::debug, Fmt, Ts&&...) -> AD_DEBUG<Fmt, Ts...>;

// ----------------------------------------------------------------------

template <typename Fmt, typename... Ts> struct AD_ASSERT
{
    AD_ASSERT(bool condition, Fmt format, Ts&&... ts, const acmacs::log::source_location& sl = acmacs::log::source_location{})
    {
        acmacs::log::ad_assert(condition, sl, format, std::forward<Ts>(ts)...);
    }
};

template <typename Fmt, typename... Ts> AD_ASSERT(bool, Fmt, Ts&&...) -> AD_ASSERT<Fmt, Ts...>;

// ----------------------------------------------------------------------

template <typename Fmt, typename... Ts> struct AD_FORMAT
{
    AD_FORMAT(Fmt format, Ts&&... ts, const acmacs::log::source_location& sl = acmacs::log::source_location{})
        : text{fmt::format("{} {}", acmacs::log::format(sl, format, std::forward<Ts>(ts)...), sl)}
    {
    }
    operator std::string() const { return text; }
    std::string text;
};

template <typename Fmt, typename... Ts> AD_FORMAT(Fmt, Ts&&...) -> AD_FORMAT<Fmt, Ts...>;

template <typename Fmt, typename... Ts> struct fmt::formatter<AD_FORMAT<Fmt, Ts...>> : fmt::formatter<acmacs::fmt_helper::default_formatter> {
    template <typename FormatCtx> auto format(const AD_FORMAT<Fmt, Ts...>& value, FormatCtx& ctx)
    {
        return format_to(ctx.out(), "{}", value.text);
    }
};


// ----------------------------------------------------------------------

template <typename Fmt, typename... Ts> inline void AD_PRINT(bool do_print, Fmt format, Ts&&... ts)
{
    if (do_print)
        fmt::print(stderr, format, std::forward<Ts>(ts)...);
}

template <typename Fmt, typename... Ts> inline void AD_PRINT(acmacs::debug dbg, Fmt format, Ts&&... ts)
{
    AD_PRINT(dbg == acmacs::debug::yes, format, std::forward<Ts>(ts)...);
}

template <typename Fmt, typename... Ts> inline void AD_PRINT(Fmt format, Ts&&... ts)
{
    fmt::print(stderr, format, std::forward<Ts>(ts)...);
}

// ----------------------------------------------------------------------

template <typename Fmt, typename... Ts> struct AD_LOG
{
    AD_LOG(acmacs::log::log_key_t section, Fmt format, Ts&&... ts, const acmacs::log::source_location& sl = acmacs::log::source_location{})
    {
        acmacs::log::message(section, [&]() { return acmacs::log::format(sl, format, std::forward<Ts>(ts)...); });
    }
};

template <typename Fmt, typename... Ts> AD_LOG(acmacs::log::log_key_t, Fmt, Ts&&...) -> AD_LOG<Fmt, Ts...>;

#define AD_LOG_INDENT acmacs::log::indent _indent

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
