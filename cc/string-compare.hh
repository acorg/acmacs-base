#pragma once

#include <algorithm>
#include <cctype>
#include <string_view>

// ----------------------------------------------------------------------

namespace acmacs::string
{
    inline bool endswith(std::string_view source, std::string_view suffix)
    {
        return source.size() >= suffix.size() && source.substr(source.size() - suffix.size()) == suffix;
    }

    inline bool startswith(std::string_view source, std::string_view prefix)
    {
        return source.size() >= prefix.size() && source.substr(0, prefix.size()) == prefix;
    }

    // ----------------------------------------------------------------------

    // assumes s1.size() == s2.size()
    inline bool equals_ignore_case_same_length(std::string_view s1, std::string_view s2)
    {
        for (auto c1 = s1.begin(), c2 = s2.begin(); c1 != s1.end(); ++c1, ++c2) {
            if (std::toupper(*c1) != std::toupper(*c2))
                return false;
        }
        return true;
    }

    inline bool equals_ignore_case(std::string_view s1, std::string_view s2)
    {
        return s1.size() == s2.size() && equals_ignore_case_same_length(s1, s2);
    }

    inline bool endswith_ignore_case(std::string_view source, std::string_view suffix)
    {
        return source.size() >= suffix.size() && equals_ignore_case_same_length(source.substr(source.size() - suffix.size()), suffix);
    }

    inline bool startswith_ignore_case(std::string_view source, std::string_view prefix)
    {
        return source.size() >= prefix.size() && equals_ignore_case_same_length(source.substr(0, prefix.size()), prefix);
    }

    // ----------------------------------------------------------------------

    // assumes s1.size() == s2.size()
    inline int compare_ignore_case_same_length(std::string_view s1, std::string_view s2)
    {
        for (auto c1 = s1.begin(), c2 = s2.begin(); c1 != s1.end(); ++c1, ++c2) {
            if (const auto result = std::toupper(*c1) - std::toupper(*c2); result != 0)
                return result;
        }
        return 0;
    }

    inline int compare_ignore_case(std::string_view s1, std::string_view s2)
    {
        const auto min_size = std::min(s1.size(), s2.size());
        if (const auto result = compare_ignore_case_same_length(s1.substr(0, min_size), s2.substr(0, min_size)); result != 0)
            return result;
        else
            return static_cast<int>(s1.size()) - static_cast<int>(s2.size());
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
