#pragma once

#include <string>
#include <string_view>
#include <vector>

// ----------------------------------------------------------------------

namespace acmacs::string
{
    enum class Split { RemoveEmpty, KeepEmpty };

      // http://stackoverflow.com/questions/236129/split-a-string-in-c
    inline std::vector<std::string_view> split(std::string_view s, std::string delim, Split keep_empty = Split::KeepEmpty)
    {
        std::vector<std::string_view> result;
        if (! delim.empty()) {
            for (auto substart = s.cbegin(), subend = substart; substart <= s.cend(); substart = subend + delim.size()) {
                subend = std::search(substart, s.end(), delim.begin(), delim.end());
                if (substart != subend || keep_empty == Split::KeepEmpty) {
                    result.emplace_back(substart, subend - substart);
                }
            }
        }
        else {
            result.emplace_back(s.data(), s.size());
        }
        return result;
    }

} // namespace acmacs::string

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
