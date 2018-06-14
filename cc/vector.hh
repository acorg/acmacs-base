#pragma once

#include <vector>
#include <cmath>
#include <numeric>
#include <functional>
#include <algorithm>
#include <cassert>

#include "acmacs-base/vector-math.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    class Vector : public std::vector<double>
    {
     public:
        using std::vector<double>::vector;
        Vector(const std::vector<double>& src) : std::vector<double>(src) {}
        // Vector(std::vector<double>&& src) : std::vector<double>(std::move(src)) {}
        // Vector& operator=(const std::vector<double>& src) { std::vector<double>::operator=(src); return *this; }
        // Vector& operator=(std::vector<double>&& src) { std::vector<double>::operator=(std::move(src)); return *this; }

          // add/subtruct/multiply/divide all vector elements by aAlpha
        Vector& operator+=(double aAlpha) { std::transform(begin(), end(), begin(), [aAlpha](double value) { return value + aAlpha; }); return *this; }
        Vector& operator-=(double aAlpha) { std::transform(begin(), end(), begin(), [aAlpha](double value) { return value - aAlpha; }); return *this; }
        Vector& operator*=(double aAlpha) { std::transform(begin(), end(), begin(), [aAlpha](double value) { return value * aAlpha; }); return *this; }
        Vector& operator/=(double aAlpha) { std::transform(begin(), end(), begin(), [aAlpha](double value) { return value / aAlpha; }); return *this; }

        Vector operator+(double aAlpha) const { Vector result(*this); return result += aAlpha; }
        Vector operator-(double aAlpha) const { Vector result(*this); return result -= aAlpha; }
        Vector operator*(double aAlpha) const { Vector result(*this); return result *= aAlpha; }
        Vector operator/(double aAlpha) const { Vector result(*this); return result /= aAlpha; }

        Vector& operator+=(const Vector& y) { std::transform(begin(), end(), y.begin(), begin(), std::plus<double>()); return *this; }
        Vector& operator-=(const Vector& y) { std::transform(begin(), end(), y.begin(), begin(), std::minus<double>()); return *this; }
        Vector& operator*=(const Vector& y) { std::transform(begin(), end(), y.begin(), begin(), std::multiplies<double>()); return *this; }
        Vector& operator/=(const Vector& y) { std::transform(begin(), end(), y.begin(), begin(), std::divides<double>()); return *this; }

        //   // Sets all elemets of this to zero
        // void zero() { std::fill(begin(), end(), 0.0); }

        //   // Computes the Euclidean norm ||x||_2 = \sqrt {\sum x_i^2} of this.
        // see vector-math.hh double eucledian_norm() const { return std::sqrt(std::inner_product(begin(), end(), begin(), 0.0)); }

        //   // Computes the scalar product x^T y for this and y.
        // see vector-math.hh double scalar_product(const Vector& y) const { return std::inner_product(begin(), end(), y.begin(), 0.0); }

        // see vector-math.hh double inner_product() const { return std::inner_product(begin(), end(), begin(), 0.0); }

        //   // Increments each element of y by aAlpha*this (y = alpha this + y). this unchanged.
        // see vector-math.hh void axpy(double aAlpha, Vector& y) const { std::transform(begin(), end(), y.begin(), y.begin(), [&aAlpha](double xx, double yy) { return aAlpha * xx + yy; }); }

          // Returns mean value for the elements of this.
        // double mean() const { return vector_math::mean<double>(begin(), end()); } //std::accumulate(begin(), end(), 0.0) / size(); }

          // Returns minimum value for the elements of this.
        double min() const { return *std::min_element(begin(), end()); }

          // Returns maximum value for the elements of this.
        double max() const { return *std::max_element(begin(), end()); }

          // Returns distance between this and another vector
        double distance(const Vector& aNother) const { return vector_math::distance<double>(begin(), end(), aNother.begin()); }

    }; // class Vector

    inline Vector operator+(const Vector& left, const Vector& right)
    {
        assert(left.size() == right.size());
        Vector result = left;
        result += right;
        return result;
    }

    inline Vector operator-(const Vector& left, const Vector& right)
    {
        assert(left.size() == right.size());
        Vector result = left;
        result -= right;
        return result;
    }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
