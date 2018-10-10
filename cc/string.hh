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

#include "acmacs-base/sfinae.hh"
#include "acmacs-base/to-string.hh"

// ----------------------------------------------------------------------

namespace std
{
    inline std::string operator+(std::string left, std::string_view right) { return left.append(right); }
    inline std::string operator+(std::string_view left, std::string right) { return std::string(left) + right; }
    template <typename S, typename=std::enable_if_t<acmacs::sfinae::is_const_char_or_string_view_v<S>>> inline std::string operator+(S left, std::string_view right) { return std::string(left) + right; }
    template <typename S, typename=std::enable_if_t<acmacs::sfinae::is_const_char_or_string_view_v<S>>> inline std::string operator+(S left, const char* right) { return std::string(left) + right; }

    inline unsigned long stoul(std::string_view src) { return stoul(std::string(src)); }
}

// ----------------------------------------------------------------------

namespace string
{
      // ----------------------------------------------------------------------

    inline std::string first_letter_of_words(std::string s)
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

    namespace _internal {
        template <typename InputIterator, typename Source> inline std::pair<InputIterator, InputIterator> strip_begin_end(Source& source)
        {
            auto predicate = [](auto c) { return std::isspace(c); }; // have to use lambda, other compiler cannot infer Predicate type from isspace
            auto e = std::find_if_not(source.rbegin(), source.rend(), predicate);
            auto b = std::find_if_not(source.begin(), e.base(), predicate);
            return std::make_pair(b, e.base());
        }
    } // namespace _internal

    // inline std::string& strip(std::string& source)
    // {
    //     auto be = _internal::strip_begin_end<std::string::iterator>(source);
    //     source.erase(be.second, source.end()); // erase at the end first
    //     source.erase(source.begin(), be.first); // invalidates be.second!
    //     return source;
    // }

    // inline std::string strip(std::string&& source)
    // {
    //     auto be = _internal::strip_begin_end<std::string::iterator>(source);
    //     source.erase(be.second, source.end()); // erase at the end first
    //     source.erase(source.begin(), be.first); // invalidates be.second!
    //     return source;
    // }

    inline std::string strip(std::string source)
    {
        auto be = _internal::strip_begin_end<std::string::const_iterator>(source);
        return std::string(be.first, be.second);
    }

      // ----------------------------------------------------------------------

    template <typename S, typename = std::enable_if_t<acmacs::sfinae::is_string_v<S>>> inline std::string replace(S source, std::string look_for, std::string replace_with)
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

    inline std::string replace(std::string source, char look_for, char replace_with)
    {
        for (std::string::size_type pos = source.find(look_for); pos != std::string::npos; pos = source.find(look_for, pos + 1))
            source[pos] = replace_with;
        return source;
    }

    template <typename ... Args> inline std::string replace(std::string source, std::string l1, std::string r1, std::string l2, std::string r2, Args ... args)
    {
        return replace(replace(source, l1, r1), l2, r2, args ...);
    }

    inline std::string replace_spaces(std::string source, char replacement)
    {
        std::transform(source.begin(), source.end(), source.begin(), [replacement](char c) {
            if (std::isspace(c))
                c = replacement;
            return c;
        });
        return source;
    }

    // ----------------------------------------------------------------------

    namespace _internal
    {
        template <typename Iter, typename F> inline std::string transform(Iter first, Iter last, F func)
        {
            std::string result;
            std::transform(first, last, std::back_inserter(result), func);
            return result;
        }

        template <typename Iter, typename F> inline std::string copy_if(Iter first, Iter last, F func)
        {
            std::string result;
            std::copy_if(first, last, std::back_inserter(result), func);
            return result;
        }
    }

    template <typename S> inline std::string lower(const S& source) { return _internal::transform(source.begin(), source.end(), ::tolower); }
    inline std::string lower(const char* source) { return _internal::transform(source, source + std::strlen(source), ::tolower); }
    inline std::string lower(char* source) { return lower(const_cast<const char*>(source)); }

    template <typename S> inline std::string upper(const S& source) { return _internal::transform(source.begin(), source.end(), ::toupper); }
    inline std::string upper(const char* source) { return _internal::transform(source, source + std::strlen(source), ::toupper); }
    inline std::string upper(char* source) { return upper(const_cast<const char*>(source)); }

    // inline std::string& capitalize(std::string& source)
    // {
    //     if (!source.empty()) {
    //         std::transform(source.begin(), source.begin() + 1, source.begin(), ::toupper);
    //         std::transform(source.begin() + 1, source.end(), source.begin() + 1, ::tolower);
    //     }
    //     return source;
    // }

    template <typename S> inline std::string capitalize(const S& source)
    {
        std::string result;
        if (!source.empty()) {
            std::transform(source.begin(), source.begin() + 1, std::back_inserter(result), ::toupper);
            std::transform(source.begin() + 1, source.end(), std::back_inserter(result), ::tolower);
        }
        return result;
    }

    template <typename S> inline std::string remove_spaces(const S& source) { return _internal::copy_if(source.begin(), source.end(), [](auto c) -> bool { return !std::isspace(c); }); }
    inline std::string remove_spaces(const char* source) { return remove_spaces(std::string_view(source, std::strlen(source))); }
    inline std::string remove_spaces(char* source) { return remove_spaces(std::string_view(source, std::strlen(source))); }

      // ----------------------------------------------------------------------
      // ends_with
      // ----------------------------------------------------------------------

    template <typename S> inline bool ends_with(const S& data, const char* end)
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

    inline int compare(const std::string& a, const std::string& b) { return compare(a.c_str(), a.size(), b.c_str(), b.size()); }
    inline int compare(const std::string& a, const char* b) { return compare(a.c_str(), a.size(), b, std::strlen(b)); }
    inline int compare(const char* a, const std::string& b) { return compare(a, std::strlen(a), b.c_str(), b.size()); }

    inline int compare(std::initializer_list<std::string> as, std::initializer_list<std::string> bs)
    {
        assert(as.size() == bs.size());
        for (auto ap = as.begin(), bp = bs.begin(); ap != as.end(); ++ap, ++bp) {
            if (const auto r = compare(*ap, *bp); r != 0)
                return r;
        }
        return 0;
    }

      // ----------------------------------------------------------------------
      // join
      // ----------------------------------------------------------------------

    template <typename Iterator> inline std::string join(std::string separator, Iterator first, Iterator last)
    {
        std::string result;
        if (first != last) {
              // Note last - first below does not supported for std::set
              // const size_t resulting_size = std::accumulate(first, last, separator.size() * static_cast<size_t>(last - first - 1), [](size_t acc, const std::string& n) -> size_t { return acc + n.size(); });
              // result.reserve(resulting_size);
            for ( ; first != last; ++first) {
                const auto f_s = acmacs::to_string(*first);
                if (!f_s.empty()) {
                    if (!result.empty())
                        result.append(separator);
                    result.append(f_s);
                }
            }
        }
        return result;
    }

    template <typename Iterator, typename Converter> inline std::string join(std::string separator, Iterator first, Iterator last, Converter convert)
    {
        std::string result;
        for ( ; first != last; ++first) {
            const auto f_s = convert(*first);
            if (!f_s.empty()) {
                if (!result.empty())
                    result.append(separator);
                result.append(f_s);
            }
        }
        return result;
    }

    template <typename Collection> inline std::string join(std::string separator, const Collection& values)
    {
        return join(separator, std::begin(values), std::end(values));
    }

    inline std::string join(std::string separator, std::initializer_list<std::string>&& values)
    {
        return join(separator, std::begin(values), std::end(values));
    }

    inline std::string join(std::string separator, std::initializer_list<std::string_view>&& values)
    {
        return join(separator, std::begin(values), std::end(values));
    }

    inline std::string join(std::initializer_list<std::string>&& parts)
    {
        // std::vector<std::string> p{parts};
        // return join(" ", std::begin(p), std::remove(std::begin(p), std::end(p), std::string()));
        return join(" ", std::begin(parts), std::end(parts));
    }

// ----------------------------------------------------------------------

    template <typename S> inline std::string_view string_view(const S& aSrc, size_t aOffset)
    {
        return std::string_view(aSrc.data() + aOffset, aSrc.size() - aOffset);
    }

// ----------------------------------------------------------------------

    template <typename S1, typename S2> inline size_t hamming_distance(S1&& first, S2&& second)
    {
        const auto size = std::min(first.size(), second.size());
        size_t dist = 0;
        for (size_t index = 0; index < size; ++index) {
            if (first[index] != second[index])
                ++dist;
        }
        return dist;
    }

// ----------------------------------------------------------------------

} // namespace string

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
