#include <vector>

#include "acmacs-base/debug.hh"
#include "acmacs-base/date.hh"

// ----------------------------------------------------------------------

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
        auto today = date::today();
        fmt::print("{}\n", today);
        using pairs_t = std::vector<std::pair<const char*, date::year_month_day>>;
        for (auto& [source, expected] : pairs_t{{"05/04/17", 2017_y/5/4}, {"10/10/11", 2011_y/10/10}, {"9/11/2001", 2001_y/9/11}, {"9/11/88", 1988_y/9/11}, {"September 11, 2001", 2001_y/9/11}, {"September 11 2001", 2001_y/9/11}, {"Sep 11 2001", 2001_y/9/11}, {"2001-09-11", 2001_y/9/11}, {"13/10/11", 2011_y/10/13}}) {
            const auto dat = date::from_string(source);
            my_assert(dat == expected, std::string(source) + " -> " + date::display(dat) + " != " + date::display(expected));
        }
        const auto for_months_ago = 2018_y/8/8;
        my_assert(2018_y/2/8 == date::months_ago(for_months_ago, 6), "6 months ago for " + date::display(for_months_ago) + " ?-> " + date::display(date::months_ago(for_months_ago, 6)));
        const auto for_years_ago = 2018_y/8/8;
        my_assert(2012_y/8/8 == date::years_ago(for_years_ago, 6), "6 years ago for " + date::display(for_years_ago) + " ?-> " + date::display(date::years_ago(for_years_ago, 6)));
        const auto for_weeks_ago = 2018_y/8/8;
        my_assert(2018_y/6/27 == date::weeks_ago(for_weeks_ago, 6), "6 weeks ago for " + date::display(for_weeks_ago) + " ?-> " + date::display(date::weeks_ago(for_weeks_ago, 6)));
        const auto for_beginning_of_week = 2018_y/8/8;
        my_assert(2018_y/8/6 == date::beginning_of_week(for_beginning_of_week), "beginning of week for " + date::display(for_beginning_of_week) + " ?-> " + date::display(date::beginning_of_week(for_beginning_of_week)));
        const auto for_next_week = 2018_y/7/30;
        my_assert(2018_y/8/6 == date::next_week(for_next_week), "next week for " + date::display(for_next_week) + " ?-> " + date::display(date::next_week(for_next_week)));
        const auto m1{2018_y/7/30}, m2{2017_y/8/10};
        my_assert(date::months_between_dates(m1, m2) == -11, "months_between_dates(2018, 2017)");
        my_assert(date::months_between_dates(m2, m1) == 11, "months_between_dates(2017, 2018)");

        const auto dat0 = date::from_string("2019-00-00", date::allow_incomplete::yes, date::throw_on_error::yes);
        fmt::print(date::display(dat0, date::allow_incomplete::yes));

    }
    catch (std::exception& err) {
        fmt::print(stderr, "{}\n", err);
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
