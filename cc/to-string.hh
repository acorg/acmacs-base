#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cstdio>
#include <cmath>

// ----------------------------------------------------------------------

namespace acmacs
{
    // template <typename T> inline std::string to_string(T aSrc)
    // {
    //     return std::to_string(aSrc);
    // }

    inline std::string to_string(int src) { return std::to_string(src); }
    inline std::string to_string(unsigned src) { return std::to_string(src); }
    inline std::string to_string(long src) { return std::to_string(src); }
    inline std::string to_string(unsigned long src) { return std::to_string(src); }
    inline std::string to_string(float src) { return std::to_string(src); }

    inline std::string to_string(double value, int precision = 32)
    {
        const auto num_digits_before_dot = static_cast<int>(std::log10(std::abs(value))) + 1;
        constexpr const size_t buffer_size = 100;
        char buffer[buffer_size + 1];
        const int written = std::snprintf(buffer, buffer_size, "%.*g", precision + num_digits_before_dot, value);
        if (written < 0 && static_cast<size_t>(written) >= buffer_size)
            throw std::runtime_error("acmacs::to_string(double) internal error");
        return {buffer, static_cast<size_t>(written)};
    }

    inline std::string to_string(std::string src) { return src; }
    inline std::string to_string(std::string_view src) { return std::string(src); }
    inline std::string to_string(const char* src) { return src; }

    template <typename Iter> inline std::string to_string(Iter first, Iter last)
    {
        std::string result{'['};
        while (first != last) {
            if (result.size() > 1)
                result += ", ";
            result += to_string(acmacs::to_string(*first));
            ++first;
        }
        result += "]";
        return result;
    }

    template <typename Element> inline std::string to_string(const std::vector<Element>& src)
    {
        return to_string(src.begin(), src.end());
    }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
