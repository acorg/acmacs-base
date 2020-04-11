#pragma once

#include "acmacs-base/string-from-chars.hh"

// ----------------------------------------------------------------------

namespace in_json
{
    inline namespace v1
    {
        template <typename T> struct in
        {
            std::string_view value;

            in() = default;
            in(std::string_view source) : value{source} {}
            in& operator=(std::string_view source) { value = source; return *this; }
            operator T() const { return acmacs::string::from_chars<T>(value); }
        };
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
