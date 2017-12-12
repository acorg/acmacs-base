#pragma once

#include <string>
#include <cstdio>

// ----------------------------------------------------------------------

namespace acmacs
{
    template <typename T> inline std::string to_string(T aSrc)
    {
        return std::to_string(aSrc);
    }

    template <> inline std::string to_string(const std::string& src) { return src; }
    template <> inline std::string to_string(std::string src) { return src; }
    template <> inline std::string to_string(const std::string_view& src) { return std::string(src); }
    template <> inline std::string to_string(std::string_view src) { return std::string(src); }
    template <> inline std::string to_string(const char* src) { return src; }

    template <> inline std::string to_string(double value)
    {
        constexpr const size_t buffer_size = 100;
        char buffer[buffer_size + 1];
        const int written = snprintf(buffer, buffer_size, "%.32g", value);
        if (written < 0 && static_cast<size_t>(written) >= buffer_size)
            throw std::runtime_error("acmacs::to_string(double) internal error");
        return {buffer, static_cast<size_t>(written)};
    }

    template <typename Element> inline std::string to_string(const std::vector<Element>& src)
    {
        std::string result{'['};
        for (const auto& elt: src) {
            if (result.size() > 1)
                result += ", ";
            result += to_string(elt);
        }
        result += "]";
        return result;
    }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
