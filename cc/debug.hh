#pragma once

#include <cmath>
#include <cstdlib>

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
#define AD_DEBUG_IF(dbg, ...) acmacs::log::debug_if(dbg, fmt::format(__VA_ARGS__), __FILE__, __LINE__)

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
        enum {
            timer,
            settings,
            vaccines
        };

        namespace detail_message
        {
            using section_t = uint32_t;

            extern section_t enabled;
            extern size_t indent;
            constexpr size_t indent_size{4};

            inline bool is_enabled(section_t section) { return section & enabled; }

            void register_enabler(std::string_view name, section_t value);
            std::string section_names(section_t section);

            template <typename Enum> section_t to_section_t(Enum value)
            {
                if (static_cast<size_t>(value) > (sizeof(section_t) * 8))
                    throw std::runtime_error{fmt::format("acmacs::log::register_enabler: enum value is too big: {}", static_cast<size_t>(value))};
                return section_t{1} << static_cast<size_t>(value);
            }

            template <typename MesssageGetter> void message(section_t section, MesssageGetter get_message)
            {
                if (is_enabled(section))
                    fmt::print(stderr, ">>>> [{}]: {:{}s}{}\n", section_names(section), "", indent, get_message());
            }

        } // namespace detail_message

        template <typename Enum> void register_enabler(std::string_view name, Enum value)
        {
            detail_message::register_enabler(name, detail_message::to_section_t(value));
        }

        template <typename Enum, typename MesssageGetter> void message(Enum section, MesssageGetter get_message)
        {
            detail_message::message(detail_message::to_section_t(section), get_message);
        }

        template <typename Enum> bool is_enabled(Enum value) { return detail_message::is_enabled(detail_message::to_section_t(value)); }

        std::vector<std::string_view> registered_enablers();
        void register_enabler_acmacs_base();

        void enable(std::string_view names);
        void enable(const std::vector<std::string_view>& names);

        struct indent
        {
            indent() { detail_message::indent += detail_message::indent_size; }
            ~indent() { detail_message::indent -= detail_message::indent_size; }
        };

        // ----------------------------------------------------------------------

        namespace detail_debug
        {
            extern bool enabled;
        }

        template <typename Filename, typename LineNo> inline void debug_print(std::string_view prefix, std::string_view message, Filename filename, LineNo line_no)
        {
            if (detail_debug::enabled)
                fmt::print(stderr, "{} {} @@ {}:{}\n", prefix, message, filename, line_no);
        }

        template <typename Filename, typename LineNo> inline void debug_if(debug dbg, std::string_view message, Filename filename, LineNo line_no)
        {
            if (dbg == acmacs::debug::yes)
                fmt::print(stderr, ">>>> {} @@ {}:{}\n", message, filename, line_no);
        }

        template <typename Filename, typename LineNo> inline void ad_assert(bool condition, std::string_view message, Filename filename, LineNo line_no)
        {
            if (!(condition)) {
                fmt::print(stderr, "> ASSERTION FAILED {} @@ {}:{}\n", message, filename, line_no);
                std::abort();
            }
        }

    } // namespace log::inlinev1

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
