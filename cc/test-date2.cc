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
