#pragma once

#include <string>
#include <charconv>
#include <limits>

// ----------------------------------------------------------------------

namespace acmacs::string
{
    template <typename T, typename S> T from_chars(S&& source)
    {
        if constexpr (std::is_same_v<T, double>) { // double is not supported by std::from_chars in clang8
            return std::stod(std::string{source});
        }
        else {
            const std::string_view src{source};
            T result;
            if (const auto [p, ec] = std::from_chars(&*src.begin(), &*src.end(), result); ec == std::errc{} && p == &*src.end())
                return result;
        }
        return std::numeric_limits<T>::max();
    }

    template <typename T, typename S> T from_chars(S&& source, size_t& processed)
    {
        if constexpr (std::is_same_v<T, double>) { // double is not supported by std::from_chars in clang8
            return std::stod(std::string{source}, &processed);
        }
        else {
            const std::string_view src{source};
            T result;
            if (const auto [p, ec] = std::from_chars(&*src.begin(), &*src.end(), result); ec == std::errc{}) {
                processed = static_cast<size_t>(p - &*src.begin());
                return result;
            }
        }
        return std::numeric_limits<T>::max();
    }

    namespace detail
    {
        template <typename T> constexpr T from_chars_to_unsigned_impl(const char* source, const char** end, T value)
        {
            constexpr const auto isdigit = [](char cc) { return cc <= '9' && cc >= '0'; };
            if (isdigit(*source)) {
                return from_chars_to_unsigned_impl(source + 1, end, static_cast<T>(*source - '0') + value * T{10});
            }
            else {
                if (end)
                    *end = source;
                return value;
            }
        }

    } // namespace detail

    // https://stackoverflow.com/questions/25195176/how-do-i-convert-a-c-string-to-a-int-at-compile-time
    template <typename T> constexpr T from_chars_to_unsigned(const char* source, const char** end)
    {
        return detail::from_chars_to_unsigned_impl(source, end, T{0});
    }

} // namespace acmacs::string

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
