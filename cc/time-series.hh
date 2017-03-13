#pragma once

#include <string>

#include "acmacs-base/date.hh"

// ----------------------------------------------------------------------

class MonthlyTimeSeries
{
 public:
    inline MonthlyTimeSeries() {}
    inline MonthlyTimeSeries(const Date& aStart, const Date& aEnd) : mStart(aStart.beginning_of_month()), mEnd(aEnd.beginning_of_month()) {}
    inline MonthlyTimeSeries(std::string aStart, std::string aEnd) : mStart(Date(aStart).beginning_of_month()), mEnd(Date(aEnd).beginning_of_month()) {}

    inline int number_of_month() const { return months_between_dates(mStart, mEnd); }

 private:
    Date mStart, mEnd;

    friend inline std::ostream& operator << (std::ostream& out, const MonthlyTimeSeries& aTS)
        {
            return out << aTS.mStart << ".." << aTS.mEnd << " (" << aTS.number_of_month() << " months)";
        }

// ----------------------------------------------------------------------


}; // class MonthlyTimeSeries

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
