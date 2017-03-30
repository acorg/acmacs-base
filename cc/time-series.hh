#pragma once

#include <string>
#include <iterator>

#include "acmacs-base/date.hh"

// ----------------------------------------------------------------------

class TimeSeriesIterator : public std::iterator<std::input_iterator_tag, Date, Date, const Date*, Date>
{
 public:
    inline TimeSeriesIterator(const TimeSeriesIterator&) = default;
    virtual ~TimeSeriesIterator();

    bool operator==(const TimeSeriesIterator& other) const { return mDate == other.mDate; }
    bool operator!=(const TimeSeriesIterator& other) const {return !(*this == other);}
    TimeSeriesIterator& operator++() { mDate = next(); return *this; }
    reference operator*() const { return mDate; }
    pointer operator->() const { return &mDate; }

    virtual Date next() const = 0;
    virtual std::string numeric_name() const = 0;
    virtual std::string text_name() const { return numeric_name(); }

    std::string first_date() const { return mDate.year4_month2_day2(); }
    std::string after_last_date() const { return next().year4_month2_day2(); }

 protected:
    inline TimeSeriesIterator(const Date& d) : mDate(d) {}
    Date mDate;

}; // class TimeSeriesIterator

// ----------------------------------------------------------------------

class MonthlyTimeSeries
{
 public:
    // inline MonthlyTimeSeries() {}
    inline MonthlyTimeSeries(const Date& aStart, const Date& aEnd) : mStart(aStart.beginning_of_month()), mEnd(aEnd.beginning_of_month()) {}
    inline MonthlyTimeSeries(std::string aStart, std::string aEnd) : mStart(Date(aStart).beginning_of_month()), mEnd(Date(aEnd).beginning_of_month()) {}

    inline int number_of_months() const { return months_between_dates(mStart, mEnd); }

    class Iterator : public TimeSeriesIterator
    {
     public:
        virtual inline Date next() const { return mDate.next_month(); }
        virtual std::string numeric_name() const { return mDate.year4_month2(); }
        virtual std::string text_name() const { return mDate.monthtext_year(); }

     private:
        friend class MonthlyTimeSeries;
        inline Iterator(const Date& d) : TimeSeriesIterator(d) {}
    };

    inline Iterator begin() const { return mStart; }
    inline Iterator end() const { return mEnd; }

 private:
    Date mStart, mEnd;

    friend inline std::ostream& operator << (std::ostream& out, const MonthlyTimeSeries& aTS)
        {
            return out << aTS.mStart << ".." << aTS.mEnd << " (" << aTS.number_of_months() << " months)";
        }

}; // class MonthlyTimeSeries

// ----------------------------------------------------------------------

class YearlyTimeSeries
{
 public:
    // inline YearlyTimeSeries() {}
    inline YearlyTimeSeries(const Date& aStart, const Date& aEnd) : mStart(aStart.beginning_of_year()), mEnd(aEnd.beginning_of_year()) {}
    inline YearlyTimeSeries(std::string aStart, std::string aEnd) : mStart(Date(aStart).beginning_of_year()), mEnd(Date(aEnd).beginning_of_year()) {}

    inline int number_of_years() const { return years_between_dates(mStart, mEnd); }

    class Iterator : public TimeSeriesIterator
    {
     public:
        virtual inline Date next() const { return mDate.next_year(); }
        virtual std::string numeric_name() const { return mDate.year_4(); }

     private:
        friend class YearlyTimeSeries;
        inline Iterator(const Date& d) : TimeSeriesIterator(d) {}
    };

    inline Iterator begin() const { return mStart; }
    inline Iterator end() const { return mEnd; }

 private:
    Date mStart, mEnd;

    friend inline std::ostream& operator << (std::ostream& out, const YearlyTimeSeries& aTS)
        {
            return out << aTS.mStart << ".." << aTS.mEnd << " (" << aTS.number_of_years() << " years)";
        }

}; // class YearlyTimeSeries

// ----------------------------------------------------------------------

class WeeklyTimeSeries
{
 public:
    // inline WeeklyTimeSeries() {}
    inline WeeklyTimeSeries(const Date& aStart, const Date& aEnd) : mStart(aStart.beginning_of_week()), mEnd(aEnd.beginning_of_week()) {}
    inline WeeklyTimeSeries(std::string aStart, std::string aEnd) : mStart(Date(aStart).beginning_of_week()), mEnd(Date(aEnd).beginning_of_week()) {}

    inline int number_of_weeks() const { return weeks_between_dates(mStart, mEnd); }

    class Iterator : public TimeSeriesIterator
    {
     public:
        virtual inline Date next() const { return mDate.next_week(); }
        virtual std::string numeric_name() const { return mDate.display(); }

     private:
        friend class WeeklyTimeSeries;
        inline Iterator(const Date& d) : TimeSeriesIterator(d) {}
    };

    inline Iterator begin() const { return mStart; }
    inline Iterator end() const { return mEnd; }

 private:
    Date mStart, mEnd;

    friend inline std::ostream& operator << (std::ostream& out, const WeeklyTimeSeries& aTS)
        {
            return out << aTS.mStart << ".." << aTS.mEnd << " (" << aTS.number_of_weeks() << " weeks)";
        }

}; // class WeeklyTimeSeries

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
