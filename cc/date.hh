#pragma once

#include <charconv>

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#pragma GCC diagnostic ignored "-Wshift-sign-overflow"
#pragma GCC diagnostic ignored "-Wundef"
#endif

#include <date/date.h>
#include <date/iso_week.h>

#pragma GCC diagnostic pop

#include "acmacs-base/fmt.hh"
// #include "acmacs-base/string_view.hh"
#include "acmacs-base/sfinae.hh"

// ----------------------------------------------------------------------

namespace date
{
    class date_parse_error : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

    using period_diff_t = int;

    enum class throw_on_error { no, yes };
    enum class allow_incomplete { no, yes };

    constexpr year_month_day invalid_date() { return year{0} / 0 / 0; }

    inline year_month_day today() { return floor<days>(std::chrono::system_clock::now()); }
    inline size_t current_year() { return static_cast<size_t>(static_cast<int>(today().year())); }

    inline std::string display(const year_month_day& dt, const char* fmt = "%Y-%m-%d") { return dt.ok() ? format(fmt, dt) : std::string{"*invalid-date*"}; }

    inline std::string display(const year_month_day& dt, allow_incomplete allow)
    {
        if (dt.ok()) {
            return format("%Y-%m-%d", dt);
        }
        else if (allow == allow_incomplete::yes) {
            if (static_cast<unsigned>(dt.month()) == 0)
                return fmt::format("{}", static_cast<int>(dt.year()));
            else if (static_cast<unsigned>(dt.day()) == 0)
                return fmt::format("{}-{:02d}", static_cast<int>(dt.year()), static_cast<unsigned>(dt.month()));
            else
                return fmt::format("{}-{:02d}-{:02d}", static_cast<int>(dt.year()), static_cast<unsigned>(dt.month()), static_cast<unsigned>(dt.day()));
        }
        else
            return fmt::format("*invalid-date: {}-{}-{}*", static_cast<int>(dt.year()), static_cast<unsigned>(dt.month()), static_cast<unsigned>(dt.day()));
    }

    inline auto month_3(const year_month_day& dt) { return format("%b", dt); }
    inline auto year_2(const year_month_day& dt) { return format("%y", dt); }
    inline auto year_4(const year_month_day& dt) { return format("%Y", dt); }
    inline auto month3_year2(const year_month_day& dt) { return format("%b %y", dt); }
    inline auto monthtext_year(const year_month_day& dt) { return format("%B %Y", dt); }
    inline auto year4_month2(const year_month_day& dt) { return format("%Y-%m", dt); }
    inline auto year4_month2_day2(const year_month_day& dt) { return format("%Y-%m-%d", dt); }

    inline auto beginning_of_month(const year_month_day& dt) { unsigned mont = static_cast<unsigned>(dt.month()); if (mont == 0) mont = 7; return year_month_day(dt.year(), month{mont}, day{1}); }
    inline auto beginning_of_year(const year_month_day& dt) { return year_month_day(dt.year(), month{1}, day{1}); }
    inline auto beginning_of_week(const year_month_day& dt)
    {
        const auto make = [](const iso_week::year_weeknum_weekday& yw) { return year_month_day{iso_week::year_weeknum_weekday(yw.year(), yw.weeknum(), iso_week::weekday{Monday})}; };
        if (static_cast<unsigned>(dt.day()) == 0) {
            if (static_cast<unsigned>(dt.month()) == 0)
                return make(iso_week::year_weeknum_weekday(dt.year() / month{7} / day{1}));
            else
                return make(iso_week::year_weeknum_weekday(dt.year() / dt.month() / day{1}));
        }
        else {
            return make(iso_week::year_weeknum_weekday(dt));
        }
    }

    // returns date for the last day of the year-month stored in this
    inline auto end_of_month(const year_month_day& dt) { return year_month_day(year_month_day_last(dt.year(), month_day_last(dt.month()))); }

    inline auto months_ago(const year_month_day& dt, period_diff_t number_of_months) { return dt - date::months(number_of_months); }
    inline auto years_ago(const year_month_day& dt, period_diff_t number_of_years) { return dt - date::years(number_of_years); }
    inline auto weeks_ago(const year_month_day& dt, period_diff_t number_of_weeks) { return static_cast<date::sys_days>(dt) - date::weeks(number_of_weeks); }

    inline auto next_month(const year_month_day& dt) { return dt + date::months(1); }
    inline auto next_year(const year_month_day& dt) { return dt + date::years(1); }
    inline auto next_week(const year_month_day& dt) { return year_month_day{static_cast<sys_days>(dt) + date::weeks(1)}; }
    inline auto next_day(const year_month_day& dt) { return year_month_day{static_cast<sys_days>(dt) + date::days(1)}; }

    inline auto& increment_month(year_month_day& dt, period_diff_t number_of_months = 1) { dt += date::months(number_of_months); return dt; }
    inline auto& decrement_month(year_month_day& dt, period_diff_t number_of_months = 1) { dt -= date::months(number_of_months); return dt; }
    inline auto& increment_year(year_month_day& dt, period_diff_t number_of_years = 1) { dt += date::years(number_of_years); return dt; }
    inline auto& decrement_year(year_month_day& dt, period_diff_t number_of_years = 1) { dt -= date::years(number_of_years); return dt; }
    inline auto& increment_week(year_month_day& dt, period_diff_t number_of_weeks = 1) { dt = static_cast<date::sys_days>(dt) + date::weeks{number_of_weeks}; return dt; }
    inline auto& decrement_week(year_month_day& dt, period_diff_t number_of_weeks = 1) { dt = static_cast<date::sys_days>(dt) - date::weeks{number_of_weeks}; return dt; }

    inline year_month_day from_string(std::string_view source, const char* fmt)
    {
            year_month_day result = invalid_date();
            std::istringstream in(std::string{source});
            if (from_stream(in, fmt, result)) {
                if (result.year() < year{30})
                    result += years(2000);
                else if (result.year() < year{100})
                    result += years(1900);
            }
            return result;
    }

    inline year_month_day from_string(std::string_view source, allow_incomplete allow = allow_incomplete::no, throw_on_error toe = throw_on_error::yes)
    {
        for (const char* fmt : {"%Y-%m-%d", "%Y%m%d", "%m/%d/%Y", "%d/%m/%Y", "%B%n %d%n %Y", "%B %d,%n %Y", "%b%n %d%n %Y", "%b %d,%n %Y"}) {
            // if (const auto result = from_string(std::forward<S>(source), fmt); result.ok())
            if (const auto result = from_string(source, fmt); result.ok())
                return result;
        }
        if (allow == allow_incomplete::yes) {
            for (const char* fmt : {"%Y-00-00", "%Y-%m-00", "%Y-%m", "%Y%m", "%Y"}) {
                // date lib cannot parse incomplete date
                constexpr int invalid = 99999;
                struct tm tm;
                tm.tm_mon = tm.tm_mday = invalid;
                if (strptime(source.data(), fmt, &tm) == &*source.end()) {
                    if (tm.tm_mon == invalid)
                        return year{tm.tm_year + 1900} / 0 / 0;
                    else
                        return year{tm.tm_year + 1900} / month{static_cast<unsigned>(tm.tm_mon) + 1} / 0;
                }
            }
        }
        if (toe == throw_on_error::yes)
            throw date_parse_error(fmt::format("cannot parse date from \"{}\" (allow_incomplete: {})", source, allow == allow_incomplete::yes));
        return invalid_date();
    }

    inline year year_from_string(std::string_view source)
    {
        int yr;
        if (const auto [p, ec] = std::from_chars(&*source.begin(), &*source.end(), yr); ec == std::errc{} && p == &*source.end())
            return year{yr};
        else
            return year{9999};
    }

    inline month month_from_string(std::string_view source)
    {
        unsigned mo;
        if (const auto [p, ec] = std::from_chars(&*source.begin(), &*source.end(), mo); ec == std::errc{} && p == &*source.end())
            return month{mo};
        else
            return month{99};
    }

    inline day day_from_string(std::string_view source)
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

    inline period_diff_t days_between_dates(const year_month_day& a, const year_month_day& b) { return std::chrono::duration_cast<days>(static_cast<sys_days>(b) - static_cast<sys_days>(a)).count(); }

    inline period_diff_t weeks_between_dates(const year_month_day& a, const year_month_day& b) { return std::chrono::duration_cast<weeks>(static_cast<sys_days>(b) - static_cast<sys_days>(a)).count(); }

    inline period_diff_t months_between_dates(const year_month_day& a, const year_month_day& b) { return std::chrono::duration_cast<months>(static_cast<sys_days>(b) - static_cast<sys_days>(a)).count(); }

    inline period_diff_t calendar_months_between_dates(const year_month_day& a, const year_month_day& b)
    {
        return std::chrono::duration_cast<months>(static_cast<sys_days>(beginning_of_month(b)) - static_cast<sys_days>(beginning_of_month(a))).count();
    }

    inline period_diff_t calendar_months_between_dates_inclusive(const year_month_day& a, const year_month_day& b) { return calendar_months_between_dates(a, b) + 1; }

    inline period_diff_t years_between_dates(const year_month_day& a, const year_month_day& b) { return std::chrono::duration_cast<years>(static_cast<sys_days>(b) - static_cast<sys_days>(a)).count(); }

} // namespace date

// ----------------------------------------------------------------------

template <typename T> struct fmt::formatter<T, std::enable_if_t<std::is_base_of<date::year_month_day, T>::value, char>> : fmt::formatter<std::string> {
    template <typename FormatCtx> auto format(const date::year_month_day& dt, FormatCtx& ctx) { return fmt::formatter<std::string>::format(date::display(dt, date::allow_incomplete::yes), ctx); }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
