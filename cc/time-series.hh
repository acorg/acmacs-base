#pragma once

// #include <string_view>
#include <vector>

#include <iterator>

#include "acmacs-base/date.hh"

// ----------------------------------------------------------------------

namespace rjson::inline v2
{
    class value;
}

namespace acmacs::time_series::inline v2
{
    struct slot
    {
        date::year_month_day first, after_last;
    };

    using series = std::vector<slot>;

    enum class interval { year, month, week, day };

    interval interval_from_string(std::string_view interval_name);

    struct parameters
    {
        date::year_month_day first, after_last; // {date::invalid_date()};
        interval intervl{interval::month};
        size_t number_of_intervals{1};
    };

    series make(const parameters& param);
    series make(const rjson::value& source, const parameters& default_param);
    inline series make(date::year_month_day first, date::year_month_day after_last) { return make(parameters{first, after_last}); }
    inline series make(std::string_view first, std::string_view after_last) { return make(parameters{date::from_string(first, date::allow_incomplete::yes), date::from_string(after_last, date::allow_incomplete::yes)}); }
}

// ----------------------------------------------------------------------

// template <typename T> struct fmt::formatter<T, std::enable_if_t<std::is_base_of<date::year_month_day, T>::value, char>> : fmt::formatter<std::string> {
//     template <typename FormatCtx> auto format(const date::year_month_day& dt, FormatCtx& ctx) { return fmt::formatter<std::string>::format(date::display(dt, date::allow_incomplete::yes), ctx); }
// };


// ======================================================================

// ----------------------------------------------------------------------

class TimeSeriesIterator : public std::iterator<std::input_iterator_tag, date::year_month_day, date::year_month_day, const date::year_month_day*, date::year_month_day>
{
 public:
    TimeSeriesIterator(const TimeSeriesIterator&) = default;
    virtual ~TimeSeriesIterator() = default;

    bool operator==(const TimeSeriesIterator& other) const { return mDate == other.mDate; }
    bool operator!=(const TimeSeriesIterator& other) const {return !(*this == other);}
    TimeSeriesIterator& operator++() { mDate = next(); return *this; }
    reference operator*() const { return mDate; }
    pointer operator->() const { return &mDate; }

    virtual date::year_month_day next() const = 0;
    virtual std::string numeric_name() const = 0;
    virtual std::string text_name() const { return numeric_name(); }

    std::string first_date() const { return date::year4_month2_day2(mDate); }
    std::string after_last_date() const { return date::year4_month2_day2(next()); }

 protected:
    inline TimeSeriesIterator(const date::year_month_day& d) : mDate(d) {}
    date::year_month_day mDate;

}; // class TimeSeriesIterator

// ----------------------------------------------------------------------

class MonthlyTimeSeries
{
 public:
    MonthlyTimeSeries(const date::year_month_day& aStart, const date::year_month_day& aEnd) : mStart(date::beginning_of_month(aStart)), mEnd(date::beginning_of_month(aEnd)) {}
    MonthlyTimeSeries(std::string_view aStart, std::string_view aEnd) : mStart(date::beginning_of_month(date::from_string(aStart))), mEnd(date::beginning_of_month(date::from_string(aEnd))) {}

    int number_of_months() const { return date::months_between_dates(mStart, mEnd); }

    class Iterator : public TimeSeriesIterator
    {
     public:
        virtual date::year_month_day next() const { return date::next_month(mDate); }
        virtual std::string numeric_name() const { return date::year4_month2(mDate); }
        virtual std::string text_name() const { return date::monthtext_year(mDate); }

     private:
        friend class MonthlyTimeSeries;
        Iterator(const date::year_month_day& d) : TimeSeriesIterator(d) {}
    };

    Iterator begin() const { return mStart; }
    Iterator end() const { return mEnd; }

    const auto& m_start() const { return mStart; }
    const auto& m_end() const { return mEnd; }

 private:
    date::year_month_day mStart, mEnd;

}; // class MonthlyTimeSeries

template <> struct fmt::formatter<MonthlyTimeSeries> : fmt::formatter<std::string> {
    template <typename FormatCtx> auto format(const MonthlyTimeSeries& ts, FormatCtx& ctx) { return fmt::formatter<std::string>::format(fmt::format("{}..{} ({})", ts.m_start(), ts.m_end(), ts.number_of_months()), ctx); }
};

// ----------------------------------------------------------------------

class YearlyTimeSeries
{
 public:
    YearlyTimeSeries(const date::year_month_day& aStart, const date::year_month_day& aEnd) : mStart(date::beginning_of_year(aStart)), mEnd(date::beginning_of_year(aEnd)) {}
    YearlyTimeSeries(std::string_view aStart, std::string_view aEnd) : mStart(date::beginning_of_year(date::from_string(aStart))), mEnd(date::beginning_of_year(date::from_string(aEnd))) {}

    int number_of_years() const { return years_between_dates(mStart, mEnd); }

    class Iterator : public TimeSeriesIterator
    {
     public:
        virtual date::year_month_day next() const { return date::next_year(mDate); }
        virtual std::string numeric_name() const { return date::year_4(mDate); }

     private:
        friend class YearlyTimeSeries;
        Iterator(const date::year_month_day& d) : TimeSeriesIterator(d) {}
    };

    Iterator begin() const { return mStart; }
    Iterator end() const { return mEnd; }

    const auto& m_start() const { return mStart; }
    const auto& m_end() const { return mEnd; }

 private:
    date::year_month_day mStart, mEnd;

}; // class YearlyTimeSeries

template <> struct fmt::formatter<YearlyTimeSeries> : fmt::formatter<std::string> {
    template <typename FormatCtx> auto format(const YearlyTimeSeries& ts, FormatCtx& ctx) { return fmt::formatter<std::string>::format(fmt::format("{}..{} ({})", ts.m_start(), ts.m_end(), ts.number_of_years()), ctx); }
};

// ----------------------------------------------------------------------

class WeeklyTimeSeries
{
 public:
    // WeeklyTimeSeries() {}
    WeeklyTimeSeries(const date::year_month_day& aStart, const date::year_month_day& aEnd) : mStart(date::beginning_of_week(aStart)), mEnd(date::beginning_of_week(aEnd)) {}
    WeeklyTimeSeries(std::string_view aStart, std::string_view aEnd) : mStart(date::beginning_of_week(date::from_string(aStart))), mEnd(date::beginning_of_week(date::from_string(aEnd))) {}

    int number_of_weeks() const { return weeks_between_dates(mStart, mEnd); }

    class Iterator : public TimeSeriesIterator
    {
     public:
        virtual date::year_month_day next() const { return date::next_week(mDate); }
        virtual std::string numeric_name() const { return date::display(mDate); }

     private:
        friend class WeeklyTimeSeries;
        Iterator(const date::year_month_day& d) : TimeSeriesIterator(d) {}
    };

    Iterator begin() const { return mStart; }
    Iterator end() const { return mEnd; }

    const auto& m_start() const { return mStart; }
    const auto& m_end() const { return mEnd; }

 private:
    date::year_month_day mStart, mEnd;

}; // class WeeklyTimeSeries

template <> struct fmt::formatter<WeeklyTimeSeries> : fmt::formatter<std::string> {
    template <typename FormatCtx> auto format(const WeeklyTimeSeries& ts, FormatCtx& ctx) { return fmt::formatter<std::string>::format(fmt::format("{}..{} ({})", ts.m_start(), ts.m_end(), ts.number_of_weeks()), ctx); }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
