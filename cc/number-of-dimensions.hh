#pragma once

#include "acmacs-base/named-type.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    using number_of_dimensions_t = named_t<size_t, struct number_of_dimensions_tag>;

    constexpr inline bool operator<(size_t lh, number_of_dimensions_t rh) { return lh < *rh; }
    constexpr inline bool operator<(number_of_dimensions_t lh, size_t rh) { return *lh < rh; }
    constexpr inline bool operator>(number_of_dimensions_t lh, size_t rh) { return *lh > rh; }
    constexpr inline bool operator>=(number_of_dimensions_t lh, number_of_dimensions_t rh) { return *lh >= *rh; }
    constexpr inline size_t operator*(size_t lh, number_of_dimensions_t rh) { return lh * rh.get(); }
    constexpr inline size_t operator/(size_t lh, number_of_dimensions_t rh) { return lh / rh.get(); }
    constexpr inline size_t operator+(size_t lh, number_of_dimensions_t rh) { return lh + rh.get(); }
    inline number_of_dimensions_t& operator++(number_of_dimensions_t& rh) { ++rh.get(); return rh; }
    constexpr inline const double* operator+(const double* lh, number_of_dimensions_t rh) { return lh + rh.get(); }
    constexpr inline double* operator+(double* lh, number_of_dimensions_t rh) { return lh + rh.get(); }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
