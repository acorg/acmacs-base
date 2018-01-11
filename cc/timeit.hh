#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <chrono>

// ----------------------------------------------------------------------

namespace acmacs
{
    using timestamp_t = decltype(std::chrono::high_resolution_clock::now());
    using duration_t = std::chrono::nanoseconds;

    inline timestamp_t timestamp() { return std::chrono::high_resolution_clock::now(); }

    inline duration_t elapsed(timestamp_t start) { return std::chrono::duration_cast<duration_t>(timestamp() - start); }
      //inline auto days(duration_t duration) { return std::chrono::duration_cast<std::chrono::days>(duration); }
    // inline auto hours(duration_t duration) { return std::chrono::duration_cast<std::chrono::hours>(duration); }

    inline std::string format(duration_t duration)
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
    }

    inline double elapsed_seconds(timestamp_t start)
    {
        const auto diff = elapsed(start).count();
        const decltype(diff) sec = diff / std::nano::den;
        const decltype(diff) nanosec = diff % std::nano::den;
        return static_cast<double>(sec) + static_cast<double>(nanosec) / double{std::nano::den};
    }

} // namespace acmacs

// ----------------------------------------------------------------------

enum class report_time { No, Yes };

class Timeit
{
 public:
    inline Timeit(std::string msg, report_time aReport = report_time::Yes, std::ostream& out = std::cerr)
        : message(msg), out_stream(out), mReport(aReport), start(acmacs::timestamp()) {}

    inline ~Timeit() { report(); }

    inline void report()
        {
            if (mReport == report_time::Yes) {
                  // const auto total = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
                const auto total = std::chrono::duration_cast<std::chrono::milliseconds>(acmacs::elapsed(start)).count();
                const decltype(total) s = total / 1000;
                const decltype(total) ms = total % 1000;
                out_stream << message;
                out_stream << s << '.' << std::setw(3) << std::setfill('0') << ms << std::setw(0) << '\n';
                mReport = report_time::No;
            }
        }

    inline void message_append(std::string to_append) { message += to_append; }

 private:
    std::string message;
    std::ostream& out_stream;
    report_time mReport;
    acmacs::timestamp_t start;
};

inline report_time do_report_time(bool do_report) { return do_report ? report_time::Yes : report_time::No; }

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
