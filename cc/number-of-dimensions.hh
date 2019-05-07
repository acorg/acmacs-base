#pragma once

#include "acmacs-base/range.hh"
#include "acmacs-base/string.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    class number_of_dimensions_t
    {
      public:
        explicit constexpr number_of_dimensions_t(size_t value) : value_(value) {}

        constexpr size_t& get() noexcept { return value_; }
        constexpr size_t get() const noexcept { return value_; }
        constexpr size_t& operator*() noexcept { return value_; }
        constexpr size_t operator*() const noexcept { return value_; }
        explicit constexpr operator size_t&() noexcept { return value_; }
        explicit constexpr operator size_t() const noexcept { return value_; }

        constexpr number_of_dimensions_t& operator++() { ++value_; return *this; }
        constexpr bool operator==(number_of_dimensions_t rhs) const noexcept { return get() == rhs.get(); }
        constexpr bool operator!=(number_of_dimensions_t rhs) const noexcept { return !operator==(rhs); }
        constexpr bool operator< (number_of_dimensions_t rhs) const noexcept { return get() < rhs.get(); }
        // constexpr bool operator<=(number_of_dimensions_t rhs) const noexcept { return get() <= rhs.get(); }
        // constexpr bool operator> (number_of_dimensions_t rhs) const noexcept { return get() >  rhs.get(); }
        constexpr bool operator>=(number_of_dimensions_t rhs) const noexcept { return get() >= rhs.get(); }

      private:
        size_t value_;

    }; // class number_of_dimensions_t

    inline std::ostream& operator<<(std::ostream& out, number_of_dimensions_t nd) { return out << nd.get(); }
    inline std::string to_string(number_of_dimensions_t nd) { return acmacs::to_string(nd.get()); }

    range(int, number_of_dimensions_t) -> range<number_of_dimensions_t>;
    range(size_t, number_of_dimensions_t) -> range<number_of_dimensions_t>;
    range(number_of_dimensions_t) -> range<number_of_dimensions_t>;
    range(number_of_dimensions_t, number_of_dimensions_t) -> range<number_of_dimensions_t>;

    // constexpr inline bool operator<(size_t lh, number_of_dimensions_t rh) { return lh < *rh; }
    // constexpr inline bool operator<(number_of_dimensions_t lh, size_t rh) { return *lh < rh; }
    // constexpr inline bool operator>(number_of_dimensions_t lh, size_t rh) { return *lh > rh; }
    // constexpr inline bool operator>=(number_of_dimensions_t lh, number_of_dimensions_t rh) { return *lh >= *rh; }
    // constexpr inline size_t operator*(size_t lh, number_of_dimensions_t rh) { return lh * rh.get(); }
    // constexpr inline size_t operator/(size_t lh, number_of_dimensions_t rh) { return lh / rh.get(); }
    // constexpr inline size_t operator+(size_t lh, number_of_dimensions_t rh) { return lh + rh.get(); }
    // inline number_of_dimensions_t& operator++(number_of_dimensions_t& rh) { ++rh.get(); return rh; }
    // constexpr inline const double* operator+(const double* lh, number_of_dimensions_t rh) { return lh + rh.get(); }
    // constexpr inline double* operator+(double* lh, number_of_dimensions_t rh) { return lh + rh.get(); }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
