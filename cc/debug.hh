#pragma once

#include <cmath>
#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

#define AD_DEBUG_FILE_LINE fmt::format(" @@ {}:{}", __FILE__, __LINE__)
#define AD_DEBUG_FILE_LINE_FUNC_S fmt::format("{}:{}: {}", __FILE__, __LINE__, __PRETTY_FUNCTION__)
#define AD_DEBUG_FILE_LINE_FUNC fmt::format(" [{}]", AD_DEBUG_FILE_LINE_FUNC_S)

#define AD_LOG(section, ...) acmacs::log::message(section, [&]() { return fmt::format("{} @@ {}:{}", fmt::format(__VA_ARGS__), __FILE__, __LINE__); })
#define AD_LOGF(section, ...) acmacs::log::message(section, [&]() { return fmt::format("{} @@ {}:{} @F {}", fmt::format(__VA_ARGS__), __FILE__, __LINE__, __PRETTY_FUNCTION__); })
#define AD_LOG_INDENT acmacs::log::indent _indent

#define AD_ERROR(...) fmt::print(stderr, "> ERROR {} @@ {}:{}\n", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define AD_WARNING(...) fmt::print(stderr, ">> WARNING {} @@ {}:{}\n", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define AD_INFO(...) fmt::print(stderr, ">>> {} @@ {}:{}\n", fmt::format(__VA_ARGS__), __FILE__, __LINE__)
#define AD_DEBUG(...) fmt::print(stderr, ">>>> {} @@ {}:{}\n", fmt::format(__VA_ARGS__), __FILE__, __LINE__)

#define AD_FORMAT(...) fmt::format("{} @@ {}:{}\n", fmt::format(__VA_ARGS__), __FILE__, __LINE__)

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

        namespace detail
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
                    fmt::print(stderr, ">>>> [{}]: {:{}s}{}\n", section_names(section), "", detail::indent, get_message());
            }

        } // namespace detail

        template <typename Enum> void register_enabler(std::string_view name, Enum value)
        {
            detail::register_enabler(name, detail::to_section_t(value));
        }

        template <typename Enum, typename MesssageGetter> void message(Enum section, MesssageGetter get_message)
        {
            detail::message(detail::to_section_t(section), get_message);
        }

        template <typename Enum> bool is_enabled(Enum value) { return detail::is_enabled(detail::to_section_t(value)); }

        void register_enabler_acmacs_base();

        void enable(std::string_view names);
        void enable(const std::vector<std::string_view>& names);

        struct indent
        {
            indent() { detail::indent += detail::indent_size; }
            ~indent() { detail::indent -= detail::indent_size; }
        };

    } // namespace log::inlinev1

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
