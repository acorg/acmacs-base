#pragma once

#include <iostream>
#include <iomanip>
#include <chrono>

#include "acmacs-base/to-string.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    using clock_t = std::chrono::high_resolution_clock;
    using timestamp_t = decltype(clock_t::now());
    using duration_t = std::chrono::nanoseconds;

    inline timestamp_t timestamp() { return clock_t::now(); }

    inline duration_t elapsed(timestamp_t start) { return std::chrono::duration_cast<duration_t>(timestamp() - start); }

    std::string format(duration_t duration);
    inline std::string to_string(duration_t duration) { return format(duration); }

    inline double elapsed_seconds(timestamp_t start) { return std::chrono::duration<double>{timestamp() - start}.count(); }

} // namespace acmacs

// ----------------------------------------------------------------------

enum class report_time { No, Yes };

class Timeit
{
 public:
    inline Timeit(std::string msg, report_time aReport = report_time::Yes, std::ostream& out = std::cerr)
        : message_(msg), out_stream_(out), report_(aReport), start_{acmacs::timestamp()} {}

    inline ~Timeit() { report(); }

    inline void report()
        {
            if (report_ == report_time::Yes) {
                report_ = report_time::No;
                out_stream_ << message_ << acmacs::to_string(acmacs::elapsed(start_)) << '\n';
            }
        }

    inline void message_append(std::string to_append) { message_ += to_append; }

 private:
    std::string message_;
    std::ostream& out_stream_;
    report_time report_;
    acmacs::timestamp_t start_;
};

inline report_time do_report_time(bool do_report) { return do_report ? report_time::Yes : report_time::No; }

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
