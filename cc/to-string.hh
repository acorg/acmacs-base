#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <tuple>
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
    inline std::string to_string(long long src) { return std::to_string(src); }
    inline std::string to_string(unsigned long long src) { return std::to_string(src); }
    inline std::string to_string(float src) { return std::to_string(src); }
    inline std::string to_string(char src) { return std::string(1, src); }

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
    namespace internal
    {
        template <typename D> inline std::string to_string_double(D value, int precision, const char* format)
        {
            const auto num_digits_before_dot = static_cast<int>(std::log10(std::abs(value))) + 1;
            constexpr const size_t buffer_size = 100;
            char buffer[buffer_size + 1];
            const int written = std::snprintf(buffer, buffer_size, format, precision + num_digits_before_dot, value);
            if (written < 0 && static_cast<size_t>(written) >= buffer_size)
                throw std::runtime_error("acmacs::to_string(double) internal error");
            return {buffer, static_cast<size_t>(written)};
        }
    } // namespace internal
#pragma GCC diagnostic pop

    inline std::string to_string(double value, int precision = 32) { return internal::to_string_double(value, precision, "%.*g"); }
    inline std::string to_string(long double value, int precision = 64) { return internal::to_string_double(value, precision, "%.*Lg"); }

    inline std::string to_string(std::string src) { return src; }
    inline std::string to_string(std::string_view src) { return std::string(src); }
    inline std::string to_string(const char* src) { return src; }

    template <typename L, typename R> inline std::string to_string(const std::pair<L, R>& arg)
    {
        std::string result{'<'};
        result += to_string(arg.first);
        result += ", ";
        result += to_string(arg.second);
        result += '>';
        return result;
    }

    template <typename Iter, typename ... Args> inline std::string to_string(Iter first, Iter last, Args&& ... args)
    {
        std::string result{'['};
        while (first != last) {
            if (result.size() > 1)
                result += ", ";
            result += to_string(*first, std::forward<Args>(args) ...);
            ++first;
        }
        result += "]";
        return result;
    }

    template <typename Element, typename ... Args> inline std::string to_string(const std::vector<Element>& src, Args&& ... args)
    {
        return to_string(src.begin(), src.end(), std::forward<Args>(args) ...);
    }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
