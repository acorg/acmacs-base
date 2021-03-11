#include "acmacs-base/fmt.hh"
#include "acmacs-base/log.hh"

// ----------------------------------------------------------------------

std::vector<std::pair<std::string_view, std::string_view>> fmt::split_for_formatting(std::string_view source)
{
    std::vector<std::pair<std::string_view, std::string_view>> result;
    size_t beg{0};
    size_t inside_format{0};
    for (size_t pos{0}; pos < source.size(); ++pos) {
        switch (source[pos]) {
            case '{':
                if (pos > beg && inside_format == 0) {
                    result.push_back(std::pair{source.substr(beg, pos - beg), source.substr(beg, pos - beg)});
                    // result.push_back(std::pair{std::string_view{}, source.substr(beg, pos - beg)});
                    beg = pos;
                }
                ++inside_format;
                break;
            case '}':
                if (inside_format > 0)
                    --inside_format;
                if (inside_format == 0) {
                    const auto pattern = source.substr(beg, pos - beg + 1);
                    if (const auto end = pattern.find(':', 1); end != std::string_view::npos)
                        result.push_back(std::pair{pattern.substr(1, end - 1), pattern});
                    else
                        result.push_back(std::pair{pattern.substr(1, pattern.size() - 2), pattern});
                    beg = pos + 1;
                }
                break;
        }
    }
    if (beg < source.size())
        result.push_back(std::pair{std::string_view{}, source.substr(beg)});
    // AD_DEBUG("split_for_formatting {}", result);
    return result;

} // fmt::split_for_formatting

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
