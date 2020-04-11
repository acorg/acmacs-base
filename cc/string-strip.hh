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

    // ----------------------------------------------------------------------

    // namespace _internal {
    //     template <typename InputIterator, typename Source> inline std::pair<InputIterator, InputIterator> strip_begin_end(Source& source)
    //     {
    //         auto predicate = [](auto c) { return std::isspace(c); }; // have to use lambda, other compiler cannot infer Predicate type from isspace
    //         auto e = std::find_if_not(source.rbegin(), source.rend(), predicate);
    //         auto b = std::find_if_not(source.begin(), e.base(), predicate);
    //         return std::make_pair(b, e.base());
    //     }
    // } // namespace _internal

    // inline std::string strip(std::string source)
    // {
    //     auto be = _internal::strip_begin_end<std::string::const_iterator>(source);
    //     return std::string(be.first, be.second);
    // }

    // inline std::string_view strip(std::string_view source)
    // {
    //     auto be = _internal::strip_begin_end<std::string_view::const_iterator>(source);
    //     return std::string_view(be.first, static_cast<size_t>(be.second - be.first));
    // }

}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
