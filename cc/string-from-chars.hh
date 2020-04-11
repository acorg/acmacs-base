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

} // namespace acmacs::string

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
