#pragma once

#include <charconv>

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#pragma GCC diagnostic ignored "-Wshift-sign-overflow"
#endif

#include <date/date.h>
#include <date/iso_week.h>

#pragma GCC diagnostic pop

#include "acmacs-base/fmt.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/sfinae.hh"

// ----------------------------------------------------------------------

namespace date
{
    class date_parse_error : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

    enum class throw_on_error { no, yes };

    constexpr year_month_day invalid_date()
    {
        using namespace literals;
        return 1999_y / 99 / 99;
    }

    inline year_month_day today() { return floor<days>(std::chrono::system_clock::now()); }
    inline size_t current_year() { return static_cast<size_t>(static_cast<int>(today().year())); }
    inline std::string display(const year_month_day& dt, const char* fmt = "%Y-%m-%d") { return dt.ok() ? format(fmt, dt) : std::string{"*invalid-date*"}; }

    inline auto month_3(const year_month_day& dt) { return format("%b", dt); }
    inline auto year_2(const year_month_day& dt) { return format("%y", dt); }
    inline auto year_4(const year_month_day& dt) { return format("%Y", dt); }
    inline auto month3_year2(const year_month_day& dt) { return format("%b %y", dt); }
    inline auto monthtext_year(const year_month_day& dt) { return format("%B %Y", dt); }
    inline auto year4_month2(const year_month_day& dt) { return format("%Y-%m", dt); }
    inline auto year4_month2_day2(const year_month_day& dt) { return format("%Y-%m-%d", dt); }

    inline auto beginning_of_month(const year_month_day& dt) { return year_month_day(dt.year(), dt.month(), day{1}); }
    inline auto beginning_of_year(const year_month_day& dt) { return year_month_day(dt.year(), month{1}, day{1}); }
    inline auto beginning_of_week(const year_month_day& dt)
    {
        iso_week::year_weeknum_weekday yw(dt);
        return year_month_day{iso_week::year_weeknum_weekday(yw.year(), yw.weeknum(), iso_week::weekday{Monday})};
    }

    // returns date for the last day of the year-month stored in this
    inline auto end_of_month(const year_month_day& dt) { return year_month_day(year_month_day_last(dt.year(), month_day_last(dt.month()))); }

    inline auto months_ago(const year_month_day& dt, int number_of_months) { return dt - date::months(number_of_months); }
    inline auto years_ago(const year_month_day& dt, int number_of_years) { return dt - date::years(number_of_years); }
    inline auto weeks_ago(const year_month_day& dt, int number_of_weeks) { return static_cast<date::sys_days>(dt) - date::weeks(number_of_weeks); }

    inline auto next_month(const year_month_day& dt) { return dt + date::months(1); }
    inline auto next_year(const year_month_day& dt) { return dt + date::years(1); }
    inline auto next_week(const year_month_day& dt) { return year_month_day{static_cast<sys_days>(dt) + date::weeks(1)}; }

    inline auto& increment_month(year_month_day& dt, int number_of_months = 1) { dt += date::months(number_of_months); return dt; }
    inline auto& decrement_month(year_month_day& dt, int number_of_months = 1) { dt -= date::months(number_of_months); return dt; }
    inline auto& increment_year(year_month_day& dt, int number_of_years = 1) { dt += date::years(number_of_years); return dt; }
    inline auto& decrement_year(year_month_day& dt, int number_of_years = 1) { dt -= date::years(number_of_years); return dt; }
    inline auto& increment_week(year_month_day& dt, int number_of_weeks = 1) { dt = static_cast<date::sys_days>(dt) + date::weeks{number_of_weeks}; return dt; }
    inline auto& decrement_week(year_month_day& dt, int number_of_weeks = 1) { dt = static_cast<date::sys_days>(dt) - date::weeks{number_of_weeks}; return dt; }

    template <typename S> inline year_month_day from_string(S&& source, const char* fmt)
    {
        year_month_day result;
        std::istringstream in(std::string{source});
        if (from_stream(in, fmt, result)) {
            if (result.year() < year{30})
                result += years(2000);
            else if (result.year() < year{100})
                result += years(1900);
        }
        else {
            result = invalid_date();
        }
        return result;
    }

    template <typename S> inline year_month_day from_string(S&& source, throw_on_error toe = throw_on_error::yes)
    {
        for (const char* fmt : {"%Y-%m-%d", "%Y%m%d", "%m/%d/%Y", "%d/%m/%Y", "%B%n %d%n %Y", "%B %d,%n %Y", "%b%n %d%n %Y", "%b %d,%n %Y"}) {
            if (const auto result = from_string(std::forward<S>(source), fmt); result.ok())
                return result;
        }
        if (toe == throw_on_error::yes)
            throw date_parse_error(fmt::format("cannot parse date from \"{}\"", source));
        return invalid_date();
    }

    template <typename S> inline year year_from_string(S&& source)
    {
        int yr;
        if (const auto [p, ec] = std::from_chars(&*source.begin(), &*source.end(), yr); ec == std::errc{} && p == &*source.end())
            return year{yr};
        else
            return year{9999};
    }

    template <typename S> inline month month_from_string(S&& source)
    {
        unsigned mo;
        if (const auto [p, ec] = std::from_chars(&*source.begin(), &*source.end(), mo); ec == std::errc{} && p == &*source.end())
            return month{mo};
        else
            return month{99};
    }

    template <typename S> inline day day_from_string(S&& source)
    {
        unsigned dy;
        if (const auto [p, ec] = std::from_chars(&*source.begin(), &*source.end(), dy); ec == std::errc{} && p == &*source.end())
            return day{dy};
        else
            return day{99};
    }

    inline std::string current_date_time()
    {
        const auto now = std::chrono::system_clock::now();
        const auto in_time_t = std::chrono::system_clock::to_time_t(now);
        return fmt::format("{:%Y-%m-%d %H:%M:%S %Z}", *std::localtime(&in_time_t));
    }

    inline int days_between_dates(const year_month_day& a, const year_month_day& b) { return std::chrono::duration_cast<days>(static_cast<sys_days>(b) - static_cast<sys_days>(a)).count(); }

    inline int weeks_between_dates(const year_month_day& a, const year_month_day& b) { return std::chrono::duration_cast<weeks>(static_cast<sys_days>(b) - static_cast<sys_days>(a)).count(); }

    inline int months_between_dates(const year_month_day& a, const year_month_day& b) { return std::chrono::duration_cast<months>(static_cast<sys_days>(b) - static_cast<sys_days>(a)).count(); }

    inline int calendar_months_between_dates(const year_month_day& a, const year_month_day& b)
    {
        return std::chrono::duration_cast<months>(static_cast<sys_days>(beginning_of_month(b)) - static_cast<sys_days>(beginning_of_month(a))).count();
    }

    inline int calendar_months_between_dates_inclusive(const year_month_day& a, const year_month_day& b) { return calendar_months_between_dates(a, b) + 1; }

    inline int years_between_dates(const year_month_day& a, const year_month_day& b) { return std::chrono::duration_cast<years>(static_cast<sys_days>(b) - static_cast<sys_days>(a)).count(); }

} // namespace date

// ----------------------------------------------------------------------

template <typename T> struct fmt::formatter<T, std::enable_if_t<std::is_base_of<date::year_month_day, T>::value, char>> : fmt::formatter<std::string> {
    template <typename FormatCtx> auto format(const date::year_month_day& dt, FormatCtx& ctx) { return fmt::formatter<std::string>::format(date::display(dt), ctx); }
};


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
