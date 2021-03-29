#include "acmacs-base/timeit.hh"

// ----------------------------------------------------------------------

std::string acmacs::format_duration(acmacs::duration_t duration)
{
    fmt::memory_buffer out;
    bool written{false};
    const auto format_val = [&out, &written](auto val, const char* terminator, bool write_zero = false) {
        if (val || write_zero || written) {
            fmt::format_to(out, "{:02d}{}", val, terminator);
            written = true;
        }
    };
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration).count();
    if (hours > 24) {
        format_val(hours / 24, "d:");
        hours %= 24;
    }
    format_val(hours, ":");
    format_val(std::chrono::duration_cast<std::chrono::minutes>(duration % std::chrono::hours(1)).count(), ":");
    format_val(std::chrono::duration_cast<std::chrono::seconds>(duration % std::chrono::minutes(1)).count(), ".", true);
    format_val(std::chrono::duration_cast<std::chrono::microseconds>(duration % std::chrono::seconds(1)).count(), "");
    return fmt::to_string(out);

} // acmacs::format_duration

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
