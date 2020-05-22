#pragma once
#error Obsolete

// #include <ctime>
#include <string>
#include <locale>
#include <iostream>

#pragma GCC diagnostic push
#include "acmacs-base/boost-diagnostics.hh"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

class Date
{
 public:
    enum Today { Today };

    Date() = default;
    Date(const Date&) = default;
    Date(enum Today) : mDate(boost::gregorian::day_clock::local_day()) {}
    template <typename S> Date(S&& aText) : mDate(from_string(std::string(std::forward<S>(aText)))) {}
    Date(boost::gregorian::date aBoostDate) : mDate(aBoostDate) {}
    static Date today() { return Today; }
    Date months_ago(int number_of_months) const { return Date(*this).decrement_month(number_of_months); }
    Date years_ago(int number_of_years) const { return Date(*this).decrement_month(number_of_years * 12); }
    Date weeks_ago(int number_of_weeks) const { return Date(*this).decrement_week(number_of_weeks); }

    Date& operator=(const Date&) = default;
    Date& operator=(std::string aText) { if (!aText.empty()) mDate = from_string(aText); return *this; }
    Date& operator=(const char* aText) { if (aText && *aText) mDate = from_string(aText); return *this; }

    bool operator < (const Date& d) const { return mDate < d.mDate; }
    bool operator == (const Date& d) const { return mDate == d.mDate; }
    bool operator >= (const Date& d) const { return mDate >= d.mDate; }

    bool empty() const { return mDate.is_not_a_date(); }
    operator bool() const { return !mDate.is_not_a_date(); }
    int year() const { return mDate.year(); }
    int month() const { return mDate.month(); }
    int day() const { return mDate.day(); }

    std::string display() const { return to_iso_extended_string(mDate); }
    operator std::string() const { return display(); }

      // returns date for the 1st day of the year-month stored in this
    Date beginning_of_month() const { return boost::gregorian::date(mDate.year(), mDate.month(), 1); }
    Date beginning_of_year() const { return boost::gregorian::date(mDate.year(), 1, 1); }
    Date beginning_of_week() const { return boost::gregorian::first_day_of_the_week_before(boost::gregorian::Monday).get_date(mDate); }

    Date& increment_month(int number_of_months = 1) { mDate += boost::gregorian::months(number_of_months); return *this; }
    Date& decrement_month(int number_of_months = 1) { mDate -= boost::gregorian::months(number_of_months); return *this; }
    Date& increment_year(int number_of_years = 1) { mDate += boost::gregorian::years(number_of_years); return *this; }
    Date& decrement_year(int number_of_years = 1) { mDate -= boost::gregorian::years(number_of_years); return *this; }
    Date& increment_week(int number_of_weeks = 1) { mDate += boost::gregorian::weeks(number_of_weeks); return *this; }
    Date& decrement_week(int number_of_weeks = 1) { mDate -= boost::gregorian::weeks(number_of_weeks); return *this; }

    Date next_month() const { return mDate + boost::gregorian::months(1); }
    Date next_year() const { return mDate + boost::gregorian::years(1); }
    Date next_week() const { return mDate + boost::gregorian::weeks(1); }

    std::string month_3() const { return format("%b"); }
    std::string year_2() const { return format("%y"); }
    std::string year_4() const { return format("%Y"); }
    std::string month3_year2() const { return format("%b %y"); }
    std::string monthtext_year() const { return format("%B %Y"); }
    std::string year4_month2() const { return format("%Y-%m"); }
    std::string year4_month2_day2() const { return format("%Y-%m-%d"); }

    const auto& gregorian() const { return mDate; }

 private:
    boost::gregorian::date mDate;

    std::string format(const char* fmt) const
        {
            std::ostringstream stream;
            stream.imbue(std::locale(std::locale::classic(), new boost::gregorian::date_facet(fmt)));
            stream << mDate;
            return stream.str();
        }

    static inline boost::gregorian::date from_string(std::string source)
        {
            try {
                return boost::gregorian::from_string(source);
            }
            catch (std::exception&) {
                return boost::gregorian::from_undelimited_string(source);
            }
        }

}; // class Date

// ----------------------------------------------------------------------

inline std::ostream& operator << (std::ostream& out, const Date& aDate)
{
    return out << aDate.display();
}

// ----------------------------------------------------------------------

// returns negative if b is earlier than a
inline int months_between_dates(const Date& a, const Date& b)
{
    int months = 0;
    if (a && b) {
        if (b < a) {
            months = - months_between_dates(b, a);
        }
        else if (a < b) {
            Date aa = a;
            aa.increment_month(1);
            while (aa < b) {
                aa.increment_month(1);
                ++months;
            }
        }
    }
    return months;
}

inline int months_between_dates(const std::pair<Date, Date>& aDates)
{
    return months_between_dates(aDates.first, aDates.second);
}

// ----------------------------------------------------------------------

// returns negative if b is earlier than a
inline int years_between_dates(const Date& a, const Date& b)
{
    int years = 0;
    if (a && b) {
        if (b < a) {
            years = - years_between_dates(b, a);
        }
        else {
            Date aa = a;
            while (aa < b) {
                aa.increment_year(1);
                ++years;
            }
        }
    }
    return years;
}

inline int years_between_dates(const std::pair<Date, Date>& aDates)
{
    return years_between_dates(aDates.first, aDates.second);
}

// ----------------------------------------------------------------------

// returns negative if b is earlier than a
inline int weeks_between_dates(const Date& a, const Date& b)
{
    int weeks = 0;
    if (a && b) {
        if (b < a) {
            weeks = - weeks_between_dates(b, a);
        }
        else {
            Date aa = a;
            while (aa < b) {
                aa.increment_week(1);
                ++weeks;
            }
        }
    }
    return weeks;
}

inline int weeks_between_dates(const std::pair<Date, Date>& aDates)
{
    return weeks_between_dates(aDates.first, aDates.second);
}

// ----------------------------------------------------------------------

// returns negative if b is earlier than a
inline ssize_t days_between_dates(const Date& a, const Date& b)
{
    ssize_t days = 0;
    if (a && b) {
        if (b < a) {
            days = - days_between_dates(b, a);
        }
        else {
            try {
                days = (b.gregorian() - a.gregorian()).days();
            }
            catch (std::exception& err) {
                throw std::runtime_error("Cannot find days between \"" + a.display() + "\" and \"" + b.display() + "\": " + err.what());
            }
        }
    }
    return days;
}

// ----------------------------------------------------------------------

// inline std::string time_format(std::string aFormat, std::time_t aTime = 0)
// {
//     std::ostringstream stream;
//     stream.imbue(std::locale(std::locale::classic(), new boost::posix_time::time_facet(aFormat.c_str())));
//     if (aTime == 0)
//         stream << boost::posix_time::second_clock::local_time();
//     else
//         stream << boost::posix_time::from_time_t(aTime);
//     return stream.str();
// }

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
