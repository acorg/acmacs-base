#pragma once

#ifndef ACMACS_DATE2
#include "acmacs-base/date1.hh"
#else

#include "acmacs-base/date2.hh"

// ----------------------------------------------------------------------

class Date
{
 public:
    enum Today { Today };

    Date() : date_(date::year(9999), date::month(99), date::day(99)) {}         // invalid date by default
    Date(enum Today) : date_{date::floor<date::days>(std::chrono::system_clock::now())} {}
    Date(const char* aText) { from_string(std::string(aText)); }
    Date(std::string aText) { from_string(aText); }
    Date(std::string_view aText) { from_string(aText); }
    static inline Date today() { return Today; }
    // static inline Date months_ago(int number_of_months) { return Date(Today).decrement_month(number_of_months); }
    // static inline Date years_ago(int number_of_years) { return Date(Today).decrement_month(number_of_years * 12); }
    // static inline Date weeks_ago(int number_of_weeks) { return Date(Today).decrement_week(number_of_weeks); }

    Date& operator=(std::string aText) { if (!aText.empty()) from_string(aText); return *this; }
    Date& operator=(std::string_view aText) { if (!aText.empty()) from_string(aText); return *this; }
    Date& operator=(const char* aText) { if (aText && *aText) from_string(std::string_view(aText)); return *this; }

    bool operator < (const Date& d) const { return date_ < d.date_; }
    bool operator == (const Date& d) const { return date_ == d.date_; }
    bool operator >= (const Date& d) const { return date_ >= d.date_; }

    bool empty() const { return !date_.ok(); }
    operator bool() const { return date_.ok(); }
    auto year() const { return static_cast<int>(date_.year()); }
    auto month() const { return static_cast<unsigned>(date_.month()); }
    auto day() const { return static_cast<unsigned>(date_.day()); }

    std::string display() const { return date_.ok() ? date::format("%Y-%m-%d", date_) : std::string{"*invalid-date*"}; }
    operator std::string() const { return display(); }

      // returns date for the 1st day of the year-month stored in this
    Date beginning_of_month() const { return date::year_month_day(date_.year(), date_.month(), date::day{1}); }
    Date beginning_of_year() const { return date::year_month_day(date_.year(), date::month{1}, date::day{1}); }
    // Date beginning_of_week() const { return boost::gregorian::first_day_of_the_week_before(boost::gregorian::Monday).get_date(mDate); }

    // Date& increment_month(int number_of_months = 1) { mDate += boost::gregorian::months(number_of_months); return *this; }
    // Date& decrement_month(int number_of_months = 1) { mDate -= boost::gregorian::months(number_of_months); return *this; }
    // Date& increment_year(int number_of_years = 1) { mDate += boost::gregorian::years(number_of_years); return *this; }
    // Date& decrement_year(int number_of_years = 1) { mDate -= boost::gregorian::years(number_of_years); return *this; }
    // Date& increment_week(int number_of_weeks = 1) { mDate += boost::gregorian::weeks(number_of_weeks); return *this; }
    // Date& decrement_week(int number_of_weeks = 1) { mDate -= boost::gregorian::weeks(number_of_weeks); return *this; }

    // Date next_month() const { return mDate + boost::gregorian::months(1); }
    // Date next_year() const { return mDate + boost::gregorian::years(1); }
    // Date next_week() const { return mDate + boost::gregorian::weeks(1); }

    // std::string month_3() const { return format("%b"); }
    // std::string year_2() const { return format("%y"); }
    // std::string year_4() const { return format("%Y"); }
    // std::string month3_year2() const { return format("%b %y"); }
    // std::string monthtext_year() const { return format("%B %Y"); }
    // std::string year4_month2() const { return format("%Y-%m"); }
    // std::string year4_month2_day2() const { return format("%Y-%m-%d"); }

      //? const auto& gregorian() const { return date_; }

 private:
    date::year_month_day date_;

    Date(const date::year_month_day& src) : date_(src) {}

    void from_string(std::string_view source)
        {
            from_string(std::string(source));
        }

    void from_string(std::string source)
        {
            for (const char* format : {"%Y-%m-%d", "%m/%d/%Y", "%d/%m/%Y", "%b %d %Y", "%D", "%b %d, %Y"}) {
                std::istringstream in(source);
                in >> date::parse(format, date_);
                if (in) {
                    if (date_.year() < date::year{30})
                        date_ += date::years(2000);
                    else if (date_.year() < date::year{100})
                        date_ += date::years(1900);
                    break;
                }
            }
        }

}; // class Date

// ----------------------------------------------------------------------

inline std::ostream& operator << (std::ostream& out, const Date& aDate)
{
    return out << aDate.display();
}

// ----------------------------------------------------------------------

// // returns negative if b is earlier than a
// inline int months_between_dates(const Date& a, const Date& b)
// {
//     int months = 0;
//     if (a && b) {
//         if (b < a) {
//             months = - months_between_dates(b, a);
//         }
//         else if (a < b) {
//             Date aa = a;
//             aa.increment_month(1);
//             while (aa < b) {
//                 aa.increment_month(1);
//                 ++months;
//             }
//         }
//     }
//     return months;
// }

// inline int months_between_dates(const std::pair<Date, Date>& aDates)
// {
//     return months_between_dates(aDates.first, aDates.second);
// }

// ----------------------------------------------------------------------

// // returns negative if b is earlier than a
// inline int years_between_dates(const Date& a, const Date& b)
// {
//     int years = 0;
//     if (a && b) {
//         if (b < a) {
//             years = - years_between_dates(b, a);
//         }
//         else {
//             Date aa = a;
//             while (aa < b) {
//                 aa.increment_year(1);
//                 ++years;
//             }
//         }
//     }
//     return years;
// }

// inline int years_between_dates(const std::pair<Date, Date>& aDates)
// {
//     return years_between_dates(aDates.first, aDates.second);
// }

// ----------------------------------------------------------------------

// // returns negative if b is earlier than a
// inline int weeks_between_dates(const Date& a, const Date& b)
// {
//     int weeks = 0;
//     if (a && b) {
//         if (b < a) {
//             weeks = - weeks_between_dates(b, a);
//         }
//         else {
//             Date aa = a;
//             while (aa < b) {
//                 aa.increment_week(1);
//                 ++weeks;
//             }
//         }
//     }
//     return weeks;
// }

// inline int weeks_between_dates(const std::pair<Date, Date>& aDates)
// {
//     return weeks_between_dates(aDates.first, aDates.second);
// }

// ----------------------------------------------------------------------

// // returns negative if b is earlier than a
// inline ssize_t days_between_dates(const Date& a, const Date& b)
// {
//     ssize_t days = 0;
//     if (a && b) {
//         if (b < a) {
//             days = - days_between_dates(b, a);
//         }
//         else {
//             try {
//                 days = (b.gregorian() - a.gregorian()).days();
//             }
//             catch (std::exception& err) {
//                 throw std::runtime_error("Cannot find days between \"" + a.display() + "\" and \"" + b.display() + "\": " + err.what());
//             }
//         }
//     }
//     return days;
// }

// ----------------------------------------------------------------------

#endif  // ACMACS_DATE2

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
