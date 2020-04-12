#pragma once

#include "acmacs-base/sfinae.hh"
#include "acmacs-base/debug.hh"

// ----------------------------------------------------------------------

namespace acmacs::string
{
    template <typename Arg1, typename Arg2, typename... Args>
    std::enable_if_t<!acmacs::sfinae::is_iterator_v<Arg1> || acmacs::sfinae::is_string_v<Arg1>, std::string> join(std::string_view separator, Arg1&& arg1, Arg2&& arg2, Args&&... rest)
    {
        if constexpr (sizeof...(rest) == 0) {
            if constexpr (acmacs::sfinae::is_string_v<Arg1>) {
                const std::string_view asv1{arg1};
                if constexpr (acmacs::sfinae::is_string_v<Arg2>) {
                    if (const std::string_view asv2{arg2}; !asv1.empty()) {
                        if (!asv2.empty()) {
                            std::string res(asv1.size() + asv2.size() + separator.size(), '#');
                            // AD_DEBUG("join strings 1:{} 2:{} sep:{} -> \"{}\"", asv1.size(), asv2.size(), separator.size(), res);
                            std::copy(asv1.begin(), asv1.end(), res.begin());
                            // AD_DEBUG("  res1: \"{}\"", res);
                            std::copy(separator.begin(), separator.end(), std::next(res.begin(), static_cast<ssize_t>(asv1.size())));
                            // AD_DEBUG("  res2: \"{}\"", res);
                            std::copy(asv2.begin(), asv2.end(), std::next(res.begin(), static_cast<ssize_t>(asv1.size() + separator.size())));
                            // AD_DEBUG("  res3: \"{}\"", res);
                            return res;
                        }
                        else
                            return std::string{asv1};
                    }
                    else
                        return std::string{asv2};
                }
                else {
                    if (!asv1.empty())
                        return fmt::format("{}{}{}", asv1, separator, arg2);
                    else
                        return fmt::format("{}", arg2);
                }
            }
            else if constexpr (acmacs::sfinae::is_string_v<Arg2>) {
                if (const std::string_view asv2{arg2}; !asv2.empty())
                    return fmt::format("{}{}{}", arg1, separator, asv2);
                else
                    return fmt::format("{}", arg1);
            }
            else {
                return fmt::format("{}{}{}", arg1, separator, arg2);
            }
        }
        else {
            return join(separator, join(separator, std::forward<Arg1>(arg1), std::forward<Arg2>(arg2)), std::forward<Args>(rest)...);
        }
    }

    // ----------------------------------------------------------------------

    template <typename Iterator,
              typename = std::enable_if_t<acmacs::sfinae::is_iterator_v<Iterator> && !acmacs::sfinae::is_string_v<Iterator>>> // is_iterator_v<const char*> is true, so we need !is_string_v
    inline std::string join(std::string_view separator, Iterator first, Iterator last)
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
                            result.append(separator);
                        result.append(*first);
                    }
                }
            }
            else {
                for (; first != last; ++first) {
                    if (!result.empty())
                        result.append(separator);
                    result.append(fmt::format("{}", *first));
                }
            }
        }
        return result;
    }

    // ----------------------------------------------------------------------

    template <typename Iterator, typename Converter, typename = std::enable_if_t<acmacs::sfinae::is_iterator_v<Iterator>>>
    inline std::string join(std::string_view separator, Iterator first, Iterator last, Converter convert)
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

    template <typename Collection> inline std::string join(std::string_view separator, const Collection& values) { return join(separator, std::begin(values), std::end(values)); }

    inline std::string join(std::string_view separator, std::initializer_list<std::string_view>&& values) { return join(separator, std::begin(values), std::end(values)); }

    inline std::string join(std::initializer_list<std::string_view>&& parts) { return join(" ", std::begin(parts), std::end(parts)); }

    // ----------------------------------------------------------------------

    template <typename ... Args> inline std::string concat(Args&& ... args)
    {
        return join("", std::forward<Args>(args) ...);
    }

} // namespace acmacs::string

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
