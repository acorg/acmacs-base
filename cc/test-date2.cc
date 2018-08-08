#include <iostream>

#include "acmacs-base/debug.hh"

#define ACMACS_DATE2
#include "acmacs-base/date.hh"

inline static void my_assert(bool condition, std::string message)
{
    if (!condition)
        throw std::runtime_error("assertion failed: " + message);
}

// ----------------------------------------------------------------------

int main()
{
    int exit_code = 0;
    try {
        Date def;
        my_assert(!def, DEBUG_LINE_FUNC_S);
        std::cout << "Date(): " << def << '\n';
        auto today = Date::today();
        std::cout << "today: " << today << '\n';
        for (const auto nine_eleven_s : {"05/04/17", "10/10/11", "9/11/2001", "9/11/88", "September 11, 2001", "September 11 2001", "2001-09-11", "13/10/11"}) {
            Date nine_eleven;
            nine_eleven = nine_eleven_s;
            std::cout << "nine_eleven: " << nine_eleven_s << " --> " << nine_eleven << '\n';
        }
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
