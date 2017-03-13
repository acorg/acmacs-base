#pragma once

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

    inline Date() : mDate() {}
    inline Date(enum Today) : mDate(boost::gregorian::day_clock::local_day()) {}
    inline Date(std::string aText) : mDate(boost::gregorian::from_string(aText)) {}
    inline Date(boost::gregorian::date aBoostDate) : mDate(aBoostDate) {}
    inline static Date today() { return Today; }
    inline static Date months_ago(int number_of_months) { return Date(Today).decrement_month(number_of_months); }
    inline static Date years_ago(int number_of_years) { return Date(Today).decrement_month(number_of_years * 12); }
    inline static Date weeks_ago(int number_of_weeks) { return Date(Today).decrement_week(number_of_weeks); }

    inline Date& operator =(std::string aText) { if (!aText.empty()) mDate = boost::gregorian::from_string(aText); return *this; }

    inline bool operator < (const Date& d) const { return mDate < d.mDate; }
    inline bool operator == (const Date& d) const { return mDate == d.mDate; }
    inline bool operator >= (const Date& d) const { return mDate >= d.mDate; }

    inline bool empty() const { return mDate.is_not_a_date(); }
    inline operator bool() const { return !mDate.is_not_a_date(); }
    inline int year() const { return mDate.year(); }
    inline int month() const { return mDate.month(); }
    inline int day() const { return mDate.day(); }

    inline std::string display() const { return to_iso_extended_string(mDate); }
    inline operator std::string() const { return display(); }

      // returns date for the 1st day of the year-month stored in this
    inline Date beginning_of_month() const { return boost::gregorian::date(mDate.year(), mDate.month(), 1); }
    inline Date beginning_of_year() const { return boost::gregorian::date(mDate.year(), 1, 1); }
    inline Date beginning_of_week() const { return boost::gregorian::first_day_of_the_week_before(boost::gregorian::Monday).get_date(mDate); }

    inline Date& increment_month(int number_of_months = 1) { mDate += boost::gregorian::months(number_of_months); return *this; }
    inline Date& decrement_month(int number_of_months = 1) { mDate -= boost::gregorian::months(number_of_months); return *this; }
    inline Date& increment_year(int number_of_years = 1) { mDate += boost::gregorian::years(number_of_years); return *this; }
    inline Date& decrement_year(int number_of_years = 1) { mDate -= boost::gregorian::years(number_of_years); return *this; }
    inline Date& increment_week(int number_of_weeks = 1) { mDate += boost::gregorian::weeks(number_of_weeks); return *this; }
    inline Date& decrement_week(int number_of_weeks = 1) { mDate -= boost::gregorian::weeks(number_of_weeks); return *this; }

    inline std::string month_3() const { return format("%b"); }
    inline std::string year_2() const { return format("%y"); }
    inline std::string month3_year2() const { return format("%b %y"); }
    inline std::string monthtext_year() const { return format("%B %Y"); }
    inline std::string year4_month2() const { return format("%Y-%m"); }

 private:
    boost::gregorian::date mDate;

    inline std::string format(const char* fmt) const
        {
            std::ostringstream stream;
            stream.imbue(std::locale(std::locale::classic(), new boost::gregorian::date_facet(fmt)));
            stream << mDate;
            return stream.str();
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
        else {
            Date aa = a;
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

inline std::string time_format(std::string aFormat, std::time_t aTime = 0)
{
    std::ostringstream stream;
    stream.imbue(std::locale(std::locale::classic(), new boost::posix_time::time_facet(aFormat.c_str())));
    if (aTime == 0)
        stream << boost::posix_time::second_clock::local_time();
    else
        stream << boost::posix_time::from_time_t(aTime);
    return stream.str();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
