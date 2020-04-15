#pragma once

#include <string_view>

namespace acmacs::string
{
    inline std::string_view strip(std::string_view source)
    {
        constexpr const char* spaces = " \t\n\r";
        const auto first = source.find_first_not_of(spaces), last = source.find_last_not_of(spaces);
        if (first == std::string_view::npos || last == std::string_view::npos)
            return {};
        return std::string_view(&source[first], last - first + 1);
    }

    inline std::string& strip_in_place(std::string& source)
    {
        constexpr const char* spaces = " \t\n\r";
        const auto first = source.find_first_not_of(spaces), last = source.find_last_not_of(spaces);
        if (first == std::string_view::npos || last == std::string_view::npos) {
            source.clear();
        }
        else {
            source.erase(std::next(source.begin(), static_cast<ssize_t>(last) + 1), source.end());
            source.erase(source.begin(), std::next(source.begin(), static_cast<ssize_t>(first)));
        }
        return source;
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
