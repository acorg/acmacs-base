#pragma once

#include <iostream>
#include <iomanip>

#include "acmacs-base/date2.hh"
#include "acmacs-base/week2.hh"

// ----------------------------------------------------------------------

class Date
{
 public:
    enum Today { Today };

    Date() : date_(date::year(9999), date::month(99), date::day(99)) {}         // invalid date by default
    Date(const Date&) = default;
    Date(enum Today) : date_{date::floor<date::days>(std::chrono::system_clock::now())} {}
    Date(const char* aText) { from_string(std::string(aText)); }
    Date(std::string aText) { from_string(aText); }
    Date(std::string_view aText) { from_string(aText); }
    Date(const date::year_month_day& src) : date_(src) {}
    Date(const date::sys_days& src) : date_(src) {}
    Date(int year, unsigned month, unsigned day) : date_(date::year(year), date::month(month), date::day(day)) {}
    static inline Date today() { return Today; }

    Date& operator=(const Date&) = default;
    Date& operator=(std::string aText) { if (!aText.empty()) from_string(aText); return *this; }
    Date& operator=(std::string_view aText) { if (!aText.empty()) from_string(aText); return *this; }
    Date& operator=(const char* aText) { if (aText && *aText) from_string(std::string_view(aText)); return *this; }

    Date months_ago(int number_of_months) const { return date_ - date::months(number_of_months); }
    Date years_ago(int number_of_years) const { return date_ - date::years(number_of_years); }
    Date weeks_ago(int number_of_weeks) const { return static_cast<date::sys_days>(date_) - date::weeks(number_of_weeks); }

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
    Date beginning_of_week() const
        {
            iso_week::year_weeknum_weekday yw(date_);
            return {iso_week::year_weeknum_weekday(yw.year(), yw.weeknum(), iso_week::weekday{date::Monday})};
        }

    Date& increment_month(int number_of_months = 1) { date_ += date::months(number_of_months); return *this; }
    Date& decrement_month(int number_of_months = 1) { date_ -= date::months(number_of_months); return *this; }
    Date& increment_year(int number_of_years = 1) { date_ += date::years(number_of_years); return *this; }
    Date& decrement_year(int number_of_years = 1) { date_ -= date::years(number_of_years); return *this; }
    Date& increment_week(int number_of_weeks = 1) { date_ = static_cast<date::sys_days>(date_) + date::weeks(number_of_weeks); return *this; }
    Date& decrement_week(int number_of_weeks = 1) { date_ = static_cast<date::sys_days>(date_) - date::weeks(number_of_weeks); return *this; }

    Date next_month() const { return date_ + date::months(1); }
    Date next_year() const { return date_ + date::years(1); }
    Date next_week() const { return {static_cast<date::sys_days>(date_) + date::weeks(1)}; }

    std::string month_3() const { return date::format("%b", date_); }
    std::string year_2() const { return date::format("%y", date_); }
    std::string year_4() const { return date::format("%Y", date_); }
    std::string month3_year2() const { return date::format("%b %y", date_); }
    std::string monthtext_year() const { return date::format("%B %Y", date_); }
    std::string year4_month2() const { return date::format("%Y-%m", date_); }
    std::string year4_month2_day2() const { return date::format("%Y-%m-%d", date_); }

      //? const auto& gregorian() const { return date_; }

 private:
    date::year_month_day date_;

    void from_string(std::string_view source)
        {
            from_string(std::string(source));
        }

    void from_string(std::string source)
        {
            using namespace date::literals;
            date_ = 1999_y/99/99;
            for (const char* format : {"%Y-%m-%d", "%m/%d/%Y", "%d/%m/%Y", "%B%n %d%n %Y", "%B %d,%n %Y", "%b%n %d%n %Y", "%b %d,%n %Y"}) {
                std::istringstream in(source);
                in >> date::parse(format, date_);
                if (in) {
                    if (date_.year() < date::year{30})
                        date_ += date::years(2000);
                    else if (date_.year() < date::year{100})
                        date_ += date::years(1900);
                    return;
                }
            }
            std::cerr << "ERROR: cannot parse date from \"" << source << "\"\n";
        }

 public:
    friend inline int days_between_dates(const Date& a, const Date& b)
        {
            return std::chrono::duration_cast<date::days>(static_cast<date::sys_days>(b.date_) - static_cast<date::sys_days>(a.date_)).count();
        }

    friend inline int weeks_between_dates(const Date& a, const Date& b)
        {
            return std::chrono::duration_cast<date::weeks>(static_cast<date::sys_days>(b.date_) - static_cast<date::sys_days>(a.date_)).count();
        }

    friend inline int months_between_dates(const Date& a, const Date& b)
        {
            return std::chrono::duration_cast<date::months>(static_cast<date::sys_days>(b.date_) - static_cast<date::sys_days>(a.date_)).count();
        }

    friend inline int years_between_dates(const Date& a, const Date& b)
        {
            return std::chrono::duration_cast<date::years>(static_cast<date::sys_days>(b.date_) - static_cast<date::sys_days>(a.date_)).count();
        }

}; // class Date

// ----------------------------------------------------------------------

inline std::ostream& operator << (std::ostream& out, const Date& aDate)
{
    return out << aDate.display();
}

// ----------------------------------------------------------------------

// https://stackoverflow.com/questions/17223096/outputting-date-and-time-in-c-using-stdchrono
inline std::string current_date_time()
{
    const auto now = std::chrono::system_clock::now();
    const auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S %Z");
    return ss.str();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
