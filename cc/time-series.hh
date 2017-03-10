#pragma once

#include <string>

#include "acmacs-base/date.hh"

// ----------------------------------------------------------------------

class MonthlyTimeSeries
{
 public:
    inline MonthlyTimeSeries() {}
    inline MonthlyTimeSeries(std::string aStart, std::string aEnd) : mStart(aStart), mEnd(aEnd) {}

 private:
    Date mStart, mEnd;

}; // class MonthlyTimeSeries

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
