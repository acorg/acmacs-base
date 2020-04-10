#pragma once

#include <string_view>
#include <algorithm>

// ----------------------------------------------------------------------

namespace acmacs::string
{
    namespace detail
    {
        template <typename Pred> inline std::string_view prefix(std::string_view source, Pred pred)
        {
            return std::string_view{source.data(), static_cast<size_t>(std::find_if(std::begin(source), std::end(source), pred) - std::begin(source))};
        }
    } // namespace detail

    inline std::string_view digit_prefix(std::string_view source)
    {
        return detail::prefix(source, [](char cc) { return !std::isdigit(cc); });
    }

    inline std::string_view non_digit_prefix(std::string_view source)
    {
        return detail::prefix(source, [](char cc) { return std::isdigit(cc); });
    }

    inline bool has_digits(std::string_view source)
    {
        return std::find_if(std::begin(source), std::end(source), [](char cc) { return std::isdigit(cc); }) != std::end(source);
    }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
