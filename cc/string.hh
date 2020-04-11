#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cctype>
#include <algorithm>
#include <cstring>
#include <iterator>
#include <utility>
#include <initializer_list>
#include <numeric>
#include <functional>
#include <cassert>
#include <limits>
#include <charconv>

#include "acmacs-base/sfinae.hh"
#include "acmacs-base/to-string.hh"

// ----------------------------------------------------------------------

namespace std
{
    inline unsigned long stoul(std::string_view src) { return stoul(std::string(src)); }
    inline unsigned long stoul(const std::string& src, int base) { return stoul(src, nullptr, base); }
    inline unsigned long stoul(std::string_view src, int base) { return stoul(std::string(src), nullptr, base); }
}

// ----------------------------------------------------------------------

namespace string
{
      // ----------------------------------------------------------------------

    inline std::string first_letter_of_words(std::string_view s)
    {
        std::string result;
        bool add = true;
        for (char c: s) {
            if (c == ' ') {
                add = true;
            }
            else if (add) {
                result.push_back(c);
                add = false;
            }
        }
        return result;
    }

      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------

    inline std::string replace(std::string_view source, std::string_view look_for, std::string_view replace_with)
    {
        std::string result;
        std::string::size_type start = 0;
        while (true) {
            const auto pos = source.find(look_for, start);
            if (pos != std::string::npos) {
                result.append(source.begin() + static_cast<std::string::difference_type>(start), source.begin() + static_cast<std::string::difference_type>(pos));
                result.append(replace_with);
                start = pos + look_for.size();
            }
            else {
                result.append(source.begin() + static_cast<std::string::difference_type>(start), source.end());
                break;
            }
        }
        return result;
    }

    inline std::string replace(std::string_view source, char look_for, char replace_with)
    {
        std::string result(source.size(), ' ');
        std::transform(std::begin(source), std::end(source), std::begin(result), [=](char c) { if (c == look_for) return replace_with; else return c; });
        return result;
    }

    template <typename ... Args> inline std::string replace(std::string_view source, std::string_view l1, std::string_view r1, std::string_view l2, std::string_view r2, Args ... args)
    {
        return replace(replace(source, l1, r1), l2, r2, args ...);
    }

    inline std::string replace_spaces(std::string_view source, char replacement)
    {
        std::string result(source.size(), replacement);
        std::transform(source.begin(), source.end(), result.begin(), [replacement](char c) {
            if (std::isspace(c))
                c = replacement;
            return c;
        });
        return result;
    }

    // ----------------------------------------------------------------------

    namespace _internal
    {
        template <typename Iter, typename F> inline std::string transform(Iter first, Iter last, F func)
        {
            std::string result(static_cast<size_t>(last - first), '?');
            std::transform(first, last, result.begin(), func);
            return result;
        }

        template <typename Iter, typename F> inline std::string copy_if(Iter first, Iter last, F func)
        {
            std::string result;
            std::copy_if(first, last, std::back_inserter(result), func);
            return result;
        }
    }

    inline std::string lower(std::string_view source) { return _internal::transform(source.begin(), source.end(), ::tolower); }
    inline std::string lower(const char* source) { return _internal::transform(source, source + std::strlen(source), ::tolower); }
    inline std::string lower(char* source) { return lower(const_cast<const char*>(source)); }
    template <typename Iter> inline std::string lower(Iter beg, Iter end) { return _internal::transform(beg, end, ::tolower); }

    inline std::string upper(std::string_view source) { return _internal::transform(source.begin(), source.end(), ::toupper); }
    inline std::string upper(const char* source) { return _internal::transform(source, source + std::strlen(source), ::toupper); }
    inline std::string upper(char* source) { return upper(const_cast<const char*>(source)); }
    template <typename Iter> inline std::string upper(Iter beg, Iter end) { return _internal::transform(beg, end, ::toupper); }

    inline std::string capitalize(std::string_view source)
    {
        std::string result;
        if (!source.empty()) {
            std::transform(source.begin(), source.begin() + 1, std::back_inserter(result), ::toupper);
            std::transform(source.begin() + 1, source.end(), std::back_inserter(result), ::tolower);
        }
        return result;
    }

    inline std::string remove_spaces(std::string_view source) { return _internal::copy_if(source.begin(), source.end(), [](auto c) -> bool { return !std::isspace(c); }); }
    inline std::string remove_spaces(const char* source) { return remove_spaces(std::string_view(source, std::strlen(source))); }
    inline std::string remove_spaces(char* source) { return remove_spaces(std::string_view(source, std::strlen(source))); }

    // changes subsequent spaces into one space
    inline std::string collapse_spaces(std::string_view source)
    {
        return _internal::copy_if(source.begin(), source.end(), [prev_was_space=false](auto c) mutable -> bool {
            const auto space = std::isspace(c);
            const auto result = !(prev_was_space && space);
            prev_was_space = space;
            return result;
        });
    }
    inline std::string collapse_spaces(const char* source) { return collapse_spaces(std::string_view(source, std::strlen(source))); }
    inline std::string collapse_spaces(char* source) { return collapse_spaces(std::string_view(source, std::strlen(source))); }

      // ----------------------------------------------------------------------
      // ends_with
      // ----------------------------------------------------------------------

    inline bool ends_with(std::string_view data, const char* end)
    {
        const std::string_view end_view{end};
        return std::string_view(data.data() + data.size() - end_view.size(), end_view.size()) == end_view;
    }

      // ----------------------------------------------------------------------
      // compare
      // ----------------------------------------------------------------------

    inline int compare(const char* a, size_t al, const char* b, size_t bl)
    {
        auto r = std::memcmp(a, b, std::min(al, bl));
        if (r == 0 && al != bl)
            return al < bl ? -1 : 1;
        else
            return r;
    }

    inline int compare(std::string_view s1, std::string_view s2) { return compare(s1.data(), s1.size(), s2.data(), s2.size()); }

    inline int compare(std::initializer_list<std::string_view> as, std::initializer_list<std::string_view> bs)
    {
        assert(as.size() == bs.size());
        for (auto ap = as.begin(), bp = bs.begin(); ap != as.end(); ++ap, ++bp) {
            if (const auto r = compare(*ap, *bp); r != 0)
                return r;
        }
        return 0;
    }

// ----------------------------------------------------------------------

    template <typename S> inline std::string_view string_view(const S& aSrc, size_t aOffset)
    {
        return std::string_view(aSrc.data() + aOffset, aSrc.size() - aOffset);
    }

// ----------------------------------------------------------------------

    namespace detail
    {
        template <typename S> inline std::string concat_to_string(S src, [[maybe_unused]] size_t precision = 6)
        {
            if constexpr (std::is_convertible_v<S, std::string>)
                return static_cast<std::string>(src);
            else if constexpr (std::is_same_v<S, double> || std::is_same_v<S, long double>)
                return acmacs::to_string(src, precision);
            else
                return acmacs::to_string(src);
        }
    }

    template <typename S1, typename S2, typename... Args> inline std::string concat(S1 s1, S2 s2, Args... args)
    {
        auto result = detail::concat_to_string(s1);
        result.append(detail::concat_to_string(s2));
        if constexpr (sizeof...(args) > 0)
            return concat(result, args...);
        else
            return result;
    }

    template <typename S1, typename S2, typename... Args> inline std::string concat_precise(S1 s1, S2 s2, Args... args)
    {
        auto result = detail::concat_to_string(s1, 32);
        result.append(detail::concat_to_string(s2, 32));
        if constexpr (sizeof...(args) > 0)
            return concat_precise(result, args...);
        else
            return result;
    }

// ----------------------------------------------------------------------

} // namespace string

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
