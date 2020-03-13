#pragma once

#include <cmath>
#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

#define DEBUG_FILE_LINE fmt::format("{}:{}", __FILE__, __LINE__)
#define DEBUG_FILE_LINE_FUNC_S fmt::format("{}:{}: {}", __FILE__, __LINE__, __PRETTY_FUNCTION__)
#define DEBUG_FILE_LINE_FUNC fmt::format(" [{}]", DEBUG_FILE_LINE_FUNC_S)

#define LOG(section, ...) acmacs::log::message(section, [&]() { return fmt::format("{} [{}]", fmt::format(__VA_ARGS__), DEBUG_FILE_LINE); })
#define LOGF(section, ...) acmacs::log::message(section, [&]() { return fmt::format("{}{}", fmt::format(__VA_ARGS__), DEBUG_FILE_LINE_FUNC); })

// ----------------------------------------------------------------------

namespace acmacs
{
    enum class debug { no, yes };
    enum class verbose { no, yes };

    template <typename Int> constexpr auto number_of_decimal_digits(Int max_value) { return static_cast<int>(std::log10(max_value)) + 1; }

    constexpr inline debug debug_from(bool verb) { return verb ? debug::yes : debug::no; }
    constexpr inline verbose verbose_from(bool verb) { return verb ? verbose::yes : verbose::no; }

    namespace log::inline v1
    {
        using section_t = uint32_t;

        template <typename MesssageGetter> void message(section_t section, MesssageGetter get_message)
        {
            if (section)
                fmt::print(stderr, "{}\n", get_message());
        }
    }

    // class debug
    // {
    //   public:
    //     debug(bool enable) : enabled_(enable), stream_(std::cerr) {}
    //     debug(bool enable, std::ostream& stream) : enabled_(enable), stream_(stream) {}

    //     template <typename T> friend inline debug& operator<<(debug& output, T&& value)
    //     {
    //         if (output.enabled_) {
    //             if (output.newline_) {
    //                 bool add_debug = true;
    //                 if constexpr (std::is_same_v<T, char>)
    //                     add_debug = value != '\n';
    //                 if (add_debug)
    //                     output.stream_ << "DEBUG: ";
    //                 output.newline_ = false;
    //             }
    //             output.stream_ << std::forward<T>(value);
    //         }
    //         if constexpr (std::is_same_v<T, char>) {
    //             if (value == '\n')
    //                 output.newline_ = true;
    //         }
    //         return output;
    //     }

    //   private:
    //     bool enabled_;
    //     std::ostream& stream_;
    //     bool newline_ = true;

    // }; // class debug

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
