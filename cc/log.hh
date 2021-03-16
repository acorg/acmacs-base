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
        struct log_key_t
        {
            explicit constexpr log_key_t(std::string_view kk) : key{kk} {}
            constexpr bool operator==(const log_key_t& rhs) const { return key == rhs.key; }
            constexpr bool operator==(std::string_view rhs) const { return key == rhs; }
            operator std::string_view() const { return key; }
            std::string_view key;
        };

        constexpr log_key_t all{"all"};
        constexpr log_key_t timer{"timer"};
        constexpr log_key_t settings{"settings"};
        constexpr log_key_t vaccines{"vaccines"};

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
            return !detail::enabled.empty() &&
                   (detail::enabled.front() == all || std::find(std::begin(detail::enabled), std::end(detail::enabled), section) != std::end(detail::enabled));
        }

        template <typename MesssageGetter> void message(log_key_t section, MesssageGetter get_message)
        {
            if (is_enabled(section))
                fmt::print(stderr, ">>>> [{}]: {:{}s}{}\n", section.key, "", detail::indent, get_message());
        }

        void enable(std::string_view names);
        inline void enable(log_key_t name) { return enable(name.key); }
        void enable(const std::vector<std::string_view>& names);

        struct indent
        {
            indent() { detail::indent += detail::indent_size; }
            ~indent() { detail::indent -= detail::indent_size; }
        };

        // ----------------------------------------------------------------------

        inline void debug_print(std::string_view prefix, std::string_view message, const char* filename, int line_no, const char* function)
        {
            if (detail::print_debug_messages)
                fmt::print(stderr, "{} {} @@ {}:{} {}\n", prefix, message, filename, line_no, function);
        }

        inline void debug_print(bool do_print, std::string_view prefix, std::string_view message, const char* filename, int line_no)
        {
            if (do_print && detail::print_debug_messages)
                fmt::print(stderr, "{} {} @@ {}:{}\n", prefix, message, filename, line_no);
        }

        inline void ad_assert(bool condition, std::string_view message, const char* filename, int line_no, const char* function)
        {
            if (!(condition)) {
                debug_print("> ASSERTION FAILED", message, filename, line_no, function);
                std::abort();
            }
        }

        inline void do_not_print_debug_messages() { detail::print_debug_messages = false; }

    } // namespace log::inline v1

} // namespace acmacs

// ======================================================================

// https://www.cppstories.com/2021/non-terminal-variadic-args/
// https://clang.llvm.org/docs/LanguageExtensions.html#source-location-builtins

template <typename Fmt, typename... Ts> struct AD_ERROR
{
    AD_ERROR(Fmt format, Ts&&... ts, const char* file = __builtin_FILE(), int line = __builtin_LINE(), const char* function = __builtin_FUNCTION())
    {
        acmacs::log::debug_print("> ERROR", fmt::format(format, std::forward<Ts>(ts)...), file, line, function);
    }
};

template <typename Fmt, typename... Ts> AD_ERROR(Fmt, Ts&&...) -> AD_ERROR<Fmt, Ts...>;

// ----------------------------------------------------------------------

template <typename Fmt, typename... Ts> struct AD_WARNING
{
    AD_WARNING(Fmt format, Ts&&... ts, const char* file = __builtin_FILE(), int line = __builtin_LINE(), const char* function = __builtin_FUNCTION())
    {
        acmacs::log::debug_print(">> WARNING", fmt::format(format, std::forward<Ts>(ts)...), file, line, function);
    }
    AD_WARNING(bool do_print, Fmt format, Ts&&... ts, const char* file = __builtin_FILE(), int line = __builtin_LINE(), const char* function = __builtin_FUNCTION())
    {
        acmacs::log::debug_print(do_print, ">> WARNING", fmt::format(format, std::forward<Ts>(ts)...), file, line, function);
    }
};

template <typename Fmt, typename... Ts> AD_WARNING(Fmt, Ts&&...) -> AD_WARNING<Fmt, Ts...>;
template <typename Fmt, typename... Ts> AD_WARNING(bool, Fmt, Ts&&...) -> AD_WARNING<Fmt, Ts...>;

// ----------------------------------------------------------------------

template <typename Fmt, typename... Ts> struct AD_INFO
{
    AD_INFO(Fmt format, Ts&&... ts, const char* file = __builtin_FILE(), int line = __builtin_LINE(), const char* function = __builtin_FUNCTION())
    {
        acmacs::log::debug_print(">>>", fmt::format(format, std::forward<Ts>(ts)...), file, line, function);
    }
    AD_INFO(bool do_print, Fmt format, Ts&&... ts, const char* file = __builtin_FILE(), int line = __builtin_LINE(), const char* function = __builtin_FUNCTION())
    {
        acmacs::log::debug_print(do_print, ">>>", fmt::format(format, std::forward<Ts>(ts)...), file, line, function);
    }
};

template <typename Fmt, typename... Ts> AD_INFO(Fmt, Ts&&...) -> AD_INFO<Fmt, Ts...>;
template <typename Fmt, typename... Ts> AD_INFO(bool, Fmt, Ts&&...) -> AD_INFO<Fmt, Ts...>;

// ----------------------------------------------------------------------

template <typename Fmt, typename... Ts> struct AD_DEBUG
{
    AD_DEBUG(Fmt format, Ts&&... ts, const char* file = __builtin_FILE(), int line = __builtin_LINE(), const char* function = __builtin_FUNCTION())
    {
        acmacs::log::debug_print(">>>>", fmt::format(format, std::forward<Ts>(ts)...), file, line, function);
    }
    AD_DEBUG(bool do_print, Fmt format, Ts&&... ts, const char* file = __builtin_FILE(), int line = __builtin_LINE(), const char* function = __builtin_FUNCTION())
    {
        acmacs::log::debug_print(do_print, ">>>>", fmt::format(format, std::forward<Ts>(ts)...), file, line, function);
    }
};

template <typename Fmt, typename... Ts> AD_DEBUG(Fmt, Ts&&...) -> AD_DEBUG<Fmt, Ts...>;
template <typename Fmt, typename... Ts> AD_DEBUG(bool, Fmt, Ts&&...) -> AD_DEBUG<Fmt, Ts...>;

// ----------------------------------------------------------------------

template <typename Fmt, typename... Ts> struct AD_ASSERT
{
    AD_ASSERT(bool condition, Fmt format, Ts&&... ts, const char* file = __builtin_FILE(), int line = __builtin_LINE(), const char* function = __builtin_FUNCTION())
    {
        acmacs::log::ad_assert(condition, fmt::format(format, std::forward<Ts>(ts)...), file, line, function);
    }
};

template <typename Fmt, typename... Ts> AD_ASSERT(bool, Fmt, Ts&&...) -> AD_ASSERT<Fmt, Ts...>;

// ----------------------------------------------------------------------

template <typename Fmt, typename... Ts> struct AD_FORMAT
{
    AD_FORMAT(Fmt format, Ts&&... ts, const char* file = __builtin_FILE(), int line = __builtin_LINE(), const char* function = __builtin_FUNCTION())
        : text{fmt::format("{} @@ {}:{} {}", fmt::format(format, std::forward<Ts>(ts)...), file, line, function)}
    {
    }
    operator std::string() const { return text; }
    std::string text;
};

template <typename Fmt, typename... Ts> AD_FORMAT(Fmt, Ts&&...) -> AD_FORMAT<Fmt, Ts...>;

// ----------------------------------------------------------------------

template <typename Fmt, typename... Ts> struct AD_LOG
{
    AD_LOG(acmacs::log::log_key_t section, Fmt format, Ts&&... ts, const char* file = __builtin_FILE(), int line = __builtin_LINE(), const char* function = __builtin_FUNCTION())
    {
        acmacs::log::message(section, [&]() { return fmt::format("{} @@ {}:{} {}", fmt::format(format, std::forward<Ts>(ts)...), file, line, function); });
    }
};

template <typename Fmt, typename... Ts> AD_LOG(acmacs::log::log_key_t, Fmt, Ts&&...) -> AD_LOG<Fmt, Ts...>;

#define AD_LOG_INDENT acmacs::log::indent _indent

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
