#pragma once

#include <cmath>
#include <cstdlib>
#include <array>

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

#define AD_DEBUG_FILE_LINE fmt::format(" @@ {}:{}", __FILE__, __LINE__)

// Unclear support for __PRETTY_FUNCTION__ by g++9
// #define AD_DEBUG_FILE_LINE_FUNC_S fmt::format("{}:{}: {}", __FILE__, __LINE__, __PRETTY_FUNCTION__)
// #define AD_DEBUG_FILE_LINE_FUNC fmt::format(" [{}]", AD_DEBUG_FILE_LINE_FUNC_S)

#define AD_LOG(section, ...) acmacs::log::message(section, [&]() { return fmt::format("{} @@ {}:{}", fmt::format(__VA_ARGS__), __FILE__, __LINE__); })
#define AD_LOGF(section, ...) acmacs::log::message(section, [&]() { return fmt::format("{} @@ {}:{} @F {}", fmt::format(__VA_ARGS__), __FILE__, __LINE__, __PRETTY_FUNCTION__); })
#define AD_LOG_INDENT acmacs::log::indent _indent

#define AD_ERROR(...) acmacs::log::debug_print("> ERROR", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define AD_WARNING(...) acmacs::log::debug_print(">> WARNING", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define AD_INFO(...) acmacs::log::debug_print(">>>", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define AD_DEBUG(...) acmacs::log::debug_print(">>>>", fmt::format(__VA_ARGS__), __FILE__, __LINE__)

#define AD_DEBUG_IF(dbg, ...) acmacs::log::debug_if(dbg, [&]() { return fmt::format(__VA_ARGS__); }, __FILE__, __LINE__)

#define AD_ASSERT(condition, ...) acmacs::log::ad_assert(condition, fmt::format(__VA_ARGS__), __FILE__, __LINE__)

#define AD_FORMAT(...) fmt::format("{} @@ {}:{}", fmt::format(__VA_ARGS__), __FILE__, __LINE__)

// ----------------------------------------------------------------------

namespace acmacs
{
    enum class debug { no, yes };
    enum class verbose { no, yes };

    template <typename Int> constexpr auto number_of_decimal_digits(Int max_value) { return static_cast<int>(std::log10(max_value)) + 1; }

    constexpr inline debug debug_from(bool verb) { return verb ? debug::yes : debug::no; }
    constexpr inline verbose verbose_from(bool verb) { return verb ? verbose::yes : verbose::no; }

    // ----------------------------------------------------------------------

    namespace log::inline v1
    {
        using log_key_t = std::string_view;

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
                fmt::print(stderr, ">>>> [{}]: {:{}s}{}\n", section, "", detail::indent, get_message());
        }

        void enable(std::string_view names);
        void enable(const std::vector<std::string_view>& names);

        struct indent
        {
            indent() { detail::indent += detail::indent_size; }
            ~indent() { detail::indent -= detail::indent_size; }
        };

        // ----------------------------------------------------------------------

        inline void debug_print(std::string_view prefix, std::string_view message, const char* filename, int line_no)
        {
            if (detail::print_debug_messages)
                fmt::print(stderr, "{} {} @@ {}:{}\n", prefix, message, filename, line_no);
        }

        template <typename MesssageGetter> void debug_if(debug do_print, MesssageGetter get_message, const char* filename, int line_no)
        {
            if (do_print == debug::yes)
                acmacs::log::debug_print(">>>>", get_message(), filename, line_no);
        }

        inline void ad_assert(bool condition, std::string_view message, const char* filename, int line_no)
        {
            if (!(condition)) {
                debug_print("> ASSERTION FAILED", message, filename, line_no);
                std::abort();
            }
        }

        inline void do_not_print_debug_messages() { detail::print_debug_messages = false; }

    } // namespace log::inline v1

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
