#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>

// ----------------------------------------------------------------------

class Timeit
{
 public:
    inline Timeit(std::string msg, std::ostream& out = std::cout)
        : message(msg), out_stream(out), start(std::chrono::steady_clock::now()) {}

    inline ~Timeit()
        {
            const auto total = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
            const decltype(total) s = total / 1000;
            const decltype(total) ms = total % 1000;
            out_stream << message << s << '.' << std::setw(3) << std::setfill('0') << ms << std::endl;
        }

 private:
    std::string message;
    std::ostream& out_stream;
    decltype(std::chrono::steady_clock::now()) start;
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
