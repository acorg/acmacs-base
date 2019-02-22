#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <limits>

#include "acmacs-base/float.hh"
#include "acmacs-base/to-string.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    class PointCoordinates
    {
      public:
        enum create_copy { create_copy };
        enum with_nan_coordinates_2D { with_nan_coordinates_2D };

        PointCoordinates(enum with_nan_coordinates_2D) : data_(2, std::numeric_limits<double>::quiet_NaN()), begin_{&*data_.begin()}, end_{&*data_.end()} {}
        PointCoordinates(size_t number_of_dimensions) : data_(number_of_dimensions, std::numeric_limits<double>::quiet_NaN()), begin_{&*data_.begin()}, end_{&*data_.end()} {}
        PointCoordinates(double x, double y) : data_{x, y}, begin_{&*data_.begin()}, end_{&*data_.end()} {}
        PointCoordinates(double x, double y, double z) : data_{x, y, z}, begin_{&*data_.begin()}, end_{&*data_.end()} {}
        PointCoordinates(enum create_copy, const PointCoordinates& source) : data_{source.begin_, source.end_}, begin_{&*data_.begin()}, end_{&*data_.end()} {}
        PointCoordinates(std::vector<double>::const_iterator first, std::vector<double>::const_iterator last) : begin_{&const_cast<double&>(*first)}, end_{&const_cast<double&>(*last)} {}
        explicit PointCoordinates(const std::vector<double>& source) : PointCoordinates(std::begin(source), std::end(source)) {}

        PointCoordinates(const PointCoordinates& rhs) : data_(rhs.data_), begin_{rhs.data_.empty() ? rhs.begin_ : &*data_.begin()}, end_{rhs.data_.empty() ? rhs.end_ : &*data_.end()} {}
        PointCoordinates(PointCoordinates&& rhs) : data_(std::move(rhs.data_)), begin_{rhs.data_.empty() ? rhs.begin_ : &*data_.begin()}, end_{rhs.data_.empty() ? rhs.end_ : &*data_.end()} {}

        PointCoordinates& operator=(const PointCoordinates& rhs)
        {
            assert(number_of_dimensions() == rhs.number_of_dimensions());
            if (rhs.data_.empty()) {
                data_.clear();
                begin_ = rhs.begin_;
                end_ = rhs.end_;
            }
            else {
                data_ = rhs.data_;
                begin_ = &*data_.begin();
                end_ = &*data_.end();
            }
            return *this;
        }

        bool operator==(const PointCoordinates& rhs) const { return std::equal(begin_, end_, rhs.begin_, rhs.end_); }
        bool operator!=(const PointCoordinates& rhs) const { return !operator==(rhs); }

        constexpr size_t number_of_dimensions() const { return static_cast<size_t>(end_ - begin_); }
        double operator[](size_t dim) const { /* assert(dim < number_of_dimensions()); */ return *(begin_ + dim); }
        double& operator[](size_t dim) { /* assert(dim < number_of_dimensions()); */ return *(begin_ + dim); }

        constexpr double x() const { return operator[](0); }
        constexpr double y() const { return operator[](1); }
        constexpr double z() const { return operator[](2); }

        void x(double val) { operator[](0) = val; }
        void y(double val) { operator[](1) = val; }
        void z(double val) { operator[](2) = val; }

        PointCoordinates operator-() const { PointCoordinates result(create_copy, *this); std::transform(begin_, end_, begin_, [](double val) { return -val; }); return result; }
        PointCoordinates operator+=(const PointCoordinates& rhs) { std::transform(begin_, end_, rhs.begin_, begin_, [](double v1, double v2) { return v1 + v2; }); return *this; }
        PointCoordinates operator+=(double val) { std::transform(begin_, end_, begin_, [val](double v1) { return v1 + val; }); return *this; }
        PointCoordinates operator-=(const PointCoordinates& rhs) { std::transform(begin_, end_, rhs.begin_, begin_, [](double v1, double v2) { return v1 - v2; }); return *this; }
        PointCoordinates operator*=(double val) { std::transform(begin_, end_, begin_, [val](double v1) { return v1 * val; }); return *this; }
        PointCoordinates operator/=(double val) { std::transform(begin_, end_, begin_, [val](double v1) { return v1 / val; }); return *this; }

        constexpr const double* begin() const { return begin_; }
        constexpr const double* end() const { return end_; }
        constexpr double* begin() { return begin_; }
        constexpr double* end() { return end_; }

        bool not_nan() const
        {
            return std::all_of(begin(), end(), [](double value) -> bool { return !std::isnan(value); });
        }

       PointCoordinates mean_with(const PointCoordinates& another) const
       {
           PointCoordinates result(number_of_dimensions());
           std::transform(begin_, end_, another.begin_, result.begin_, [](double v1, double v2) { return (v1 + v2) / 2.0; });
           return result;
       }

      private:
        std::vector<double> data_;
        double* begin_;
        double* end_;

    }; // class PointCoordinates

    inline std::string to_string(const PointCoordinates& coord, size_t precision = 32)
    {
        auto result = '{' + acmacs::to_string(coord.x(), precision) + ", " + acmacs::to_string(coord.y(), precision);
        if (coord.number_of_dimensions() == 3)
            result += ", " + to_string(coord.z(), precision);
        return result + '}';
    }

    inline std::ostream& operator<<(std::ostream& out, const PointCoordinates& coord) { return out << to_string(coord); }

    inline double distance2(const PointCoordinates& p1, const PointCoordinates& p2)
    {
        std::vector<double> diff(p1.number_of_dimensions());
        std::transform(p1.begin(), p1.end(), p2.begin(), diff.begin(), [](double v1, double v2) { return v1 - v2; });
        return std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    }

    inline double distance(const PointCoordinates& p1, const PointCoordinates& p2) { return std::sqrt(distance2(p1, p2)); }

    inline PointCoordinates operator+(const PointCoordinates& p1, const PointCoordinates& p2) { PointCoordinates result(PointCoordinates::create_copy, p1); result += p2; return result; }
    inline PointCoordinates operator+(const PointCoordinates& p1, double val) { PointCoordinates result(PointCoordinates::create_copy, p1); result += val; return result; }
    inline PointCoordinates operator-(const PointCoordinates& p1, const PointCoordinates& p2) { PointCoordinates result(PointCoordinates::create_copy, p1); result -= p2; return result; }
    inline PointCoordinates operator-(const PointCoordinates& p1, double val) { PointCoordinates result(PointCoordinates::create_copy, p1); result += -val; return result; }
    inline PointCoordinates operator*(const PointCoordinates& p1, double val) { PointCoordinates result(PointCoordinates::create_copy, p1); result *= val; return result; }
    inline PointCoordinates operator/(const PointCoordinates& p1, double val) { PointCoordinates result(PointCoordinates::create_copy, p1); result /= val; return result; }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
