#pragma once

#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

// ----------------------------------------------------------------------

inline std::string name_encode(std::string aName)
{
    static constexpr const char to_encode[] = ":;@=?[]\\";
    std::string result(aName.size() * 3, ' ');
    std::string::size_type output_pos = 0;
    for (const char src: aName) {
        if (src < '-' || std::memchr(to_encode, src, sizeof(to_encode)) != nullptr) {
            std::sprintf(&result[output_pos], "%%%02hhX", src);
            output_pos += 3;
        }
        else {
            result[output_pos++] = src;
        }
    }
    result.erase(result.begin() + static_cast<std::string::difference_type>(output_pos), result.end());
    return result;
}

// ----------------------------------------------------------------------

inline std::string name_decode(std::string aName)
{
    std::string result(aName.size(), ' ');
    std::string::size_type output_pos = 0;
    for (auto src = aName.cbegin(); src != aName.cend(); ++src) {
        if (*src == '%' && (aName.cend() - src) >= 3) {
            result[output_pos++] = static_cast<std::string::value_type>(std::strtol(&*src + 1, nullptr, 16) & 0xFF);
            src += 2;
        }
        else {
            result[output_pos++] = *src;
        }
    }
    result.erase(result.begin() + static_cast<std::string::difference_type>(output_pos), result.end());
    return result;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
