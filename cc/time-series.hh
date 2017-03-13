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

    inline int number_of_month() const { return months_between_dates(mStart, mEnd); }

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
            return out << aTS.mStart << ".." << aTS.mEnd << " (" << aTS.number_of_month() << " months)";
        }

}; // class MonthlyTimeSeries

// ----------------------------------------------------------------------

class YearlyTimeSeries
{
 public:
    inline YearlyTimeSeries() {}
    inline YearlyTimeSeries(const Date& aStart, const Date& aEnd) : mStart(aStart.beginning_of_year()), mEnd(aEnd.beginning_of_year()) {}
    inline YearlyTimeSeries(std::string aStart, std::string aEnd) : mStart(Date(aStart).beginning_of_year()), mEnd(Date(aEnd).beginning_of_year()) {}

    inline int number_of_year() const { return years_between_dates(mStart, mEnd); }

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
            return out << aTS.mStart << ".." << aTS.mEnd << " (" << aTS.number_of_year() << " years)";
        }

}; // class YearlyTimeSeries

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
