#pragma once

#include <chrono>

#include "acmacs-base/log.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    using clock_t = std::chrono::high_resolution_clock;
    using timestamp_t = decltype(clock_t::now());
    using duration_t = std::chrono::nanoseconds;

    inline timestamp_t timestamp() { return clock_t::now(); }
    inline duration_t elapsed(timestamp_t start) { return std::chrono::duration_cast<duration_t>(timestamp() - start); }
    std::string format_duration(duration_t duration);
    inline double elapsed_seconds(timestamp_t start) { return std::chrono::duration<double>{timestamp() - start}.count(); }

} // namespace acmacs

// ----------------------------------------------------------------------

enum class report_time { no, yes };

constexpr report_time do_report_time(bool do_report) { return do_report ? report_time::yes : report_time::no; }

// ----------------------------------------------------------------------

class Timeit
{
 public:
   Timeit(std::string_view msg, report_time aReport = report_time::yes, const char* file = __builtin_FILE(), int line = __builtin_LINE())
       : message_{msg}, report_(aReport), start_{acmacs::timestamp()}, file_{file}, line_{line}
   {
   }
    ~Timeit() { report(); }

    void report() const
        {
            if (report_ == report_time::yes) {
                report_ = report_time::no;
                AD_PRINT(">>>> {}: {} @@ {}:{}\n", message_, acmacs::format_duration(acmacs::elapsed(start_)), file_, line_);
            }
        }

    void message_append(const std::string& to_append) { message_ += to_append; }

 private:
    std::string message_;
    mutable report_time report_;
    acmacs::timestamp_t start_;
    const char* const file_;
    const int line_;
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
