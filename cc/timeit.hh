#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>

// ----------------------------------------------------------------------

enum class report_time { No, Yes };

class Timeit
{
 public:
    inline Timeit(std::string msg, report_time aReport = report_time::Yes, std::ostream& out = std::cerr)
        : message(msg), out_stream(out), mReport(aReport), start(std::chrono::steady_clock::now()) {}

    inline ~Timeit() { report(); }

    inline void report()
        {
            if (mReport == report_time::Yes) {
                const auto total = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
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
    decltype(std::chrono::steady_clock::now()) start;
};

inline report_time do_report_time(bool do_report) { return do_report ? report_time::Yes : report_time::No; }

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
