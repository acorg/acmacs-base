#include <sstream>

#include "acmacs-base/timeit.hh"

// ----------------------------------------------------------------------

std::string acmacs::format(acmacs::duration_t duration)
{
    std::stringstream result;
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration).count();
    if (hours > 24) {
        result << (hours / 24) << "d:";
        hours %= 24;
    }
    auto format_val = [](auto& target, auto val, char terminator) {
                          if (val || target.tellg())
                              target << std::setw(2) << std::setfill('0') << val << terminator;
                      };
    format_val(result, hours, ':');
    const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration % std::chrono::hours(1)).count();
    format_val(result, minutes, ':');
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration % std::chrono::minutes(1)).count();
    if (result.tellg())
        result << std::setw(2) << std::setfill('0');
    result << seconds << '.';
    const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration % std::chrono::seconds(1)).count();
    result << std::setw(6) << std::setfill('0') << microseconds;
    return result.str();

} // acmacs::format

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
