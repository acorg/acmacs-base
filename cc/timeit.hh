#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>

// ----------------------------------------------------------------------

class Timeit
{
 public:
    inline Timeit(std::string msg, std::ostream& out = std::cout, bool aReport = true)
        : message(msg), out_stream(out), mReport(aReport), start(std::chrono::steady_clock::now()) {}

    inline ~Timeit() { report(); }

    inline void report()
        {
            if (mReport) {
                const auto total = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
                const decltype(total) s = total / 1000;
                const decltype(total) ms = total % 1000;
                out_stream << message << s << '.' << std::setw(3) << std::setfill('0') << ms << std::endl;
                mReport = false;
            }
        }

 private:
    std::string message;
    std::ostream& out_stream;
    bool mReport;
    decltype(std::chrono::steady_clock::now()) start;
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
