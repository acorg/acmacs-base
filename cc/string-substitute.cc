#include "acmacs-base/string-substitute.hh"
#include "acmacs-base/log.hh"

static std::vector<std::string_view> split(std::string_view source);

// ----------------------------------------------------------------------

std::string acmacs::string::substitute_from_store(std::string_view pattern, const fmt::dynamic_format_arg_store<fmt::format_context>& store, if_no_substitution_found insf)
{
    const auto chunks{::split(pattern)};
    // fmt::print("{}\n", chunks);
    std::string result;
    for (const auto& chunk : chunks) {
        try {
            result.append(fmt::vformat(chunk, store));
        }
        catch (std::exception&) {
            switch (insf) {
              case if_no_substitution_found::leave_as_is:
                  result.append(chunk);
                  break;
              case if_no_substitution_found::empty:
                  break;
            }
        }
    }
    // fmt::print("\"{}\"\n", result);
    return result;

} // acmacs::string::substitute_from_store

// ----------------------------------------------------------------------

std::vector<std::string_view> split(std::string_view source)
{
    std::vector<std::string_view> result;
    size_t beg{0};
    for (size_t pos{0}; pos < source.size(); ++pos) {
        switch (source[pos]) {
            case '{':
                if (pos > 0 && source[pos - 1] != '{') {
                    result.push_back(source.substr(beg, pos - beg));
                    beg = pos;
                }
                break;
            case '}':
                if (pos < (source.size() - 1) && source[pos + 1] != '}') {
                    result.push_back(source.substr(beg, pos - beg + 1));
                    beg = pos + 1;
                }
                break;
        }
    }
    if (beg < source.size())
        result.push_back(source.substr(beg));
    return result;

} // split

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
