#pragma once

// ----------------------------------------------------------------------
// std::vector<std::string> split(const std::string& s, const std::string& delim, bool keep_empty = Split::KeepEmpty)
// Split string by delimiter into array of substrings
// std::vector<std::string> split(const std::string& s, const std::string& delim, bool keep_empty = Split::KeepEmpty)

// std::string strip(std::string source)
// Removes leading and trailing spaces from string
//
// ----------------------------------------------------------------------

#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <cstring>
#include <iterator>
#include <utility>
#include <initializer_list>
#include <numeric>
#include <sstream>
#include <iomanip>

// ----------------------------------------------------------------------

namespace string
{

// ----------------------------------------------------------------------

    using std::to_string;

    inline std::string to_string(const std::string& src) { return src; }
    inline std::string to_string(const char* src) { return src; }

// ----------------------------------------------------------------------

    enum class Split { RemoveEmpty, KeepEmpty };

      // http://stackoverflow.com/questions/236129/split-a-string-in-c
    inline std::vector<std::string> split(std::string s, std::string delim, Split keep_empty = Split::KeepEmpty)
    {
        std::vector<std::string> result;
        if (! delim.empty()) {
            for (std::string::iterator substart = s.begin(), subend = substart; substart <= s.end(); substart = subend + static_cast<std::string::difference_type>(delim.size())) {
                subend = std::search(substart, s.end(), delim.begin(), delim.end());
                if (substart != subend || keep_empty == Split::KeepEmpty) {
                    result.push_back(std::string(substart, subend));
                }
            }
        }
        else {
            result.push_back(s);
        }
        return result;
    }

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

    inline std::string replace(std::string source, std::string look_for, std::string replace_with)
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

      // ----------------------------------------------------------------------

    // inline std::string& lower(std::string& source)
    // {
    //     std::transform(source.begin(), source.end(), source.begin(), ::tolower);
    //     return source;
    // }

    inline std::string lower(std::string source)
    {
        std::string result;
        std::transform(source.begin(), source.end(), std::back_inserter(result), ::tolower);
        return result;
    }

    // inline std::string& upper(std::string& source)
    // {
    //     std::transform(source.begin(), source.end(), source.begin(), ::toupper);
    //     return source;
    // }

    inline std::string upper(std::string source)
    {
        std::string result;
        std::transform(source.begin(), source.end(), std::back_inserter(result), ::toupper);
        return result;
    }

    // inline std::string& capitalize(std::string& source)
    // {
    //     if (!source.empty()) {
    //         std::transform(source.begin(), source.begin() + 1, source.begin(), ::toupper);
    //         std::transform(source.begin() + 1, source.end(), source.begin() + 1, ::tolower);
    //     }
    //     return source;
    // }

    inline std::string capitalize(std::string source)
    {
        std::string result;
        if (!source.empty()) {
            std::transform(source.begin(), source.begin() + 1, std::back_inserter(result), ::toupper);
            std::transform(source.begin() + 1, source.end(), std::back_inserter(result), ::tolower);
        }
        return result;
    }

    template <typename Iterator> inline std::string join(std::string separator, Iterator first, Iterator last)
    {
        std::string result;
        if (first != last) {
              // Note last - first below does not supported for std::set
              // const size_t resulting_size = std::accumulate(first, last, separator.size() * static_cast<size_t>(last - first - 1), [](size_t acc, const std::string& n) -> size_t { return acc + n.size(); });
              // result.reserve(resulting_size);
            for ( ; first != last; ++first) {
                const auto f_s = to_string(*first);
                if (!f_s.empty()) {
                    if (!result.empty())
                        result.append(separator);
                    result.append(f_s);
                }
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

    inline std::string join(std::initializer_list<std::string>&& parts)
    {
        std::vector<std::string> p{parts};
        return join(" ", std::begin(p), std::remove(std::begin(p), std::end(p), std::string()));
    }

    template <typename T, typename = std::enable_if<std::is_integral<T>::value>> inline std::string to_hex_string(T aValue, bool aShowBase, bool aUppercase = true)
    {
        std::stringstream stream;
        stream << std::setfill('0') << std::setw(sizeof(aValue)*2) << std::hex;
        stream << (aShowBase ? std::showbase : std::noshowbase);
        stream << (aUppercase ? std::uppercase : std::nouppercase);
        stream << aValue;
        return stream.str();
    }

    template <typename T> inline std::string to_hex_string(const T* aPtr)
    {
        std::stringstream stream;
        const void* value = reinterpret_cast<const void*>(aPtr);
        stream << value; // std::setfill('0') << std::setw(sizeof(value)*2) << std::hex << value;
        return stream.str();
    }

// ----------------------------------------------------------------------

} // namespace string

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
