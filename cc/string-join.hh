#pragma once

#include "acmacs-base/sfinae.hh"
#include "acmacs-base/log.hh"
#include "acmacs-base/named-type.hh"

// ----------------------------------------------------------------------

namespace acmacs::string
{
    namespace detail::join
    {
        constexpr inline std::string_view stringify(std::string_view source) { return source; }
        constexpr inline std::string_view stringify(const char* source) { return source; }
        constexpr inline const std::string& stringify(const std::string& source) { return source; }

        template <typename Arg> inline std::enable_if_t<!acmacs::sfinae::is_string_v<Arg>, std::string> stringify(const Arg& arg) { return fmt::format("{}", arg); }
    } // namespace detail::join

    using join_sep_t = named_string_view_t<struct join_sep_t_tag>;
    inline const join_sep_t join_colon{":"};
    inline const join_sep_t join_comma{","};
    inline const join_sep_t join_concat{""};
    inline const join_sep_t join_dash{"-"};
    inline const join_sep_t join_newline{"\n"};
    inline const join_sep_t join_slash{"/"};
    inline const join_sep_t join_space{" "};

    template <typename Arg1, typename Arg2, typename... Args>
    inline std::enable_if_t<!acmacs::sfinae::is_iterator_v<Arg1> || acmacs::sfinae::is_string_v<Arg1>, std::string> join(join_sep_t separator, Arg1&& arg1, Arg2&& arg2, Args&&... rest)
    {
        if constexpr (sizeof...(rest) == 0) {
            const auto as1{detail::join::stringify(arg1)};
            const auto as2{detail::join::stringify(arg2)};
            if (!as1.empty()) {
                if (!as2.empty()) {
                    std::string res(as1.size() + as2.size() + separator.size(), '#');
                    std::copy(as1.begin(), as1.end(), res.begin());
                    std::copy(separator->begin(), separator->end(), std::next(res.begin(), static_cast<ssize_t>(as1.size())));
                    std::copy(as2.begin(), as2.end(), std::next(res.begin(), static_cast<ssize_t>(as1.size() + separator.size())));
                    return res;
                }
                else
                    return std::string{as1};
            }
            else
                return std::string{as2};
        }
        else {
            return join(separator, join(separator, std::forward<Arg1>(arg1), std::forward<Arg2>(arg2)), std::forward<Args>(rest)...);
        }
    }

    // ----------------------------------------------------------------------

    template <typename Iterator,
              typename = std::enable_if_t<acmacs::sfinae::is_iterator_v<Iterator> && !acmacs::sfinae::is_string_v<Iterator>>> // is_iterator_v<const char*> is true, so we need !is_string_v
    inline std::string join(join_sep_t separator, Iterator first, Iterator last)
    {
        std::string result;
        if (first != last) {
            // Note last - first below does not supported for std::set
            // const size_t resulting_size = std::accumulate(first, last, separator.size() * static_cast<size_t>(last - first - 1), [](size_t acc, const std::string& n) -> size_t { return acc +
            // n.size(); }); result.reserve(resulting_size);
            if constexpr (acmacs::sfinae::is_string_v<decltype(*first)>) {
                for (; first != last; ++first) {
                    if (!first->empty()) {
                        if (!result.empty())
                            result.append(*separator);
                        result.append(*first);
                    }
                }
            }
            else {
                for (; first != last; ++first) {
                    if (!result.empty())
                        result.append(*separator);
                    result.append(fmt::format("{}", *first));
                }
            }
        }
        return result;
    }

    // ----------------------------------------------------------------------

    template <typename Iterator, typename Converter, typename = std::enable_if_t<acmacs::sfinae::is_iterator_v<Iterator>>>
    inline std::string join(join_sep_t separator, Iterator first, Iterator last, Converter convert)
    {
        std::string result;
        for (; first != last; ++first) {
            const auto f_s = convert(*first);
            if (!f_s.empty()) {
                if (!result.empty())
                    result.append(separator);
                result.append(f_s);
            }
        }
        return result;
    }

    // ----------------------------------------------------------------------

    template <typename Collection> inline std::string join(join_sep_t separator, const Collection& values) { return join(separator, std::begin(values), std::end(values)); }

    // ----------------------------------------------------------------------

    template <typename... Args> inline std::string concat(Args&&... args) { return join(join_concat, std::forward<Args>(args)...); }

} // namespace acmacs::string

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
