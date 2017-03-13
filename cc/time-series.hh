#pragma once

#include <string>
#include <iterator>

#include "acmacs-base/date.hh"

// ----------------------------------------------------------------------

class TimeSeriesIterator : public std::iterator<std::input_iterator_tag, Date, Date, const Date*, Date>
{
 public:
    bool operator==(const TimeSeriesIterator& other) const { return mDate == other.mDate; }
    bool operator!=(const TimeSeriesIterator& other) const {return !(*this == other);}
    reference operator*() const { return mDate; }

 protected:
    inline TimeSeriesIterator(const Date& d) : mDate(d) {}
    Date mDate;

}; // class TimeSeriesIterator

// ----------------------------------------------------------------------

class MonthlyTimeSeries
{
 public:
    inline MonthlyTimeSeries() {}
    inline MonthlyTimeSeries(const Date& aStart, const Date& aEnd) : mStart(aStart.beginning_of_month()), mEnd(aEnd.beginning_of_month()) {}
    inline MonthlyTimeSeries(std::string aStart, std::string aEnd) : mStart(Date(aStart).beginning_of_month()), mEnd(Date(aEnd).beginning_of_month()) {}

    inline int number_of_months() const { return months_between_dates(mStart, mEnd); }

    class Iterator : public TimeSeriesIterator
    {
     public:
        Iterator& operator++() { mDate.increment_month(); return *this; }

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
    inline YearlyTimeSeries() {}
    inline YearlyTimeSeries(const Date& aStart, const Date& aEnd) : mStart(aStart.beginning_of_year()), mEnd(aEnd.beginning_of_year()) {}
    inline YearlyTimeSeries(std::string aStart, std::string aEnd) : mStart(Date(aStart).beginning_of_year()), mEnd(Date(aEnd).beginning_of_year()) {}

    inline int number_of_years() const { return years_between_dates(mStart, mEnd); }

    class Iterator : public TimeSeriesIterator
    {
     public:
        Iterator& operator++() { mDate.increment_year(); return *this; }

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
    inline WeeklyTimeSeries() {}
    inline WeeklyTimeSeries(const Date& aStart, const Date& aEnd) : mStart(aStart.beginning_of_week()), mEnd(aEnd.beginning_of_week()) {}
    inline WeeklyTimeSeries(std::string aStart, std::string aEnd) : mStart(Date(aStart).beginning_of_week()), mEnd(Date(aEnd).beginning_of_week()) {}

    inline int number_of_weeks() const { return weeks_between_dates(mStart, mEnd); }

    class Iterator : public TimeSeriesIterator
    {
     public:
        Iterator& operator++() { mDate.increment_week(); return *this; }

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
