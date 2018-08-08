#include <iostream>
#include <vector>

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
    using namespace date::literals;
    int exit_code = 0;
    try {
        Date def;
        my_assert(!def, DEBUG_LINE_FUNC_S);
          //std::cout << "Date(): " << def << '\n';
        auto today = Date::today();
        std::cout << "today: " << today << '\n';
        using pairs_t = std::vector<std::pair<const char*, date::year_month_day>>;
        for (auto& [source, expected] : pairs_t{{"05/04/17", 2017_y/5/4}, {"10/10/11", 2011_y/10/10}, {"9/11/2001", 2001_y/9/11}, {"9/11/88", 1988_y/9/11}, {"September 11, 2001", 2001_y/9/11}, {"September 11 2001", 2001_y/9/11}, {"2001-09-11", 2001_y/9/11}, {"13/10/11", 2011_y/10/13}}) {
            Date dat(source), exp(expected);
            my_assert(dat == exp, dat.display() + " != " + exp.display());
              // std::cout << "nine_eleven: " << nine_eleven_s << " --> " << nine_eleven << '\n';
        }
        Date for_months_ago(2018, 8, 8);
        my_assert(Date(2018, 2, 8) == for_months_ago.months_ago(6), "6 months ago for " + for_months_ago.display() + " ?-> " + for_months_ago.months_ago(6).display());
        Date for_years_ago(2018, 8, 8);
        my_assert(Date(2012, 8, 8) == for_years_ago.years_ago(6), "6 years ago for " + for_years_ago.display() + " ?-> " + for_years_ago.years_ago(6).display());
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
