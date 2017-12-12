#pragma once

#include <vector>
#include <cmath>
#include <numeric>
#include <functional>
#include <algorithm>

// ----------------------------------------------------------------------

namespace acmacs
{
    class Vector : public std::vector<double>
    {
     public:
        using std::vector<double>::vector;
        inline Vector(const std::vector<double>& src) : std::vector<double>(src) {}
        // inline Vector(std::vector<double>&& src) : std::vector<double>(std::move(src)) {}
        // inline Vector& operator=(const std::vector<double>& src) { std::vector<double>::operator=(src); return *this; }
        // inline Vector& operator=(std::vector<double>&& src) { std::vector<double>::operator=(std::move(src)); return *this; }

          // Multiplies all vector elements by aAlpha
        inline Vector& operator+=(double aAlpha) { std::transform(begin(), end(), begin(), [aAlpha](double value) { return value + aAlpha; }); return *this; }
        inline Vector& operator-=(double aAlpha) { std::transform(begin(), end(), begin(), [aAlpha](double value) { return value - aAlpha; }); return *this; }
        inline Vector& operator*=(double aAlpha) { std::transform(begin(), end(), begin(), [aAlpha](double value) { return value * aAlpha; }); return *this; }
        inline Vector& operator/=(double aAlpha) { std::transform(begin(), end(), begin(), [aAlpha](double value) { return value / aAlpha; }); return *this; }

        inline Vector& operator+=(const Vector& y) { std::transform(begin(), end(), y.begin(), begin(), std::plus<double>()); return *this; }
        inline Vector& operator-=(const Vector& y) { std::transform(begin(), end(), y.begin(), begin(), std::minus<double>()); return *this; }
        inline Vector& operator*=(const Vector& y) { std::transform(begin(), end(), y.begin(), begin(), std::multiplies<double>()); return *this; }
        inline Vector& operator/=(const Vector& y) { std::transform(begin(), end(), y.begin(), begin(), std::divides<double>()); return *this; }

          // Sets all elemets of this to zero
        inline void zero() { std::fill(begin(), end(), 0.0); }

          // Computes the Euclidean norm ||x||_2 = \sqrt {\sum x_i^2} of this.
        inline double eucledian_norm() const { return std::sqrt(std::inner_product(begin(), end(), begin(), 0.0)); }

          // Computes the scalar product x^T y for this and y.
        inline double scalar_product(const Vector& y) const { return std::inner_product(begin(), end(), y.begin(), 0.0); }

        inline double inner_product() const { return std::inner_product(begin(), end(), begin(), 0.0); }

          // Increments each element of y by aAlpha*this (y = alpha this + y). this unchanged.
        inline void axpy(double aAlpha, Vector& y) const { std::transform(begin(), end(), y.begin(), y.begin(), [&aAlpha](double xx, double yy) { return aAlpha * xx + yy; }); }

          // Returns mean value for the elements of this.
        inline double mean() const { return std::accumulate(begin(), end(), 0.0) / size(); }

          // Returns minimum value for the elements of this.
        inline double min() const { return *std::min_element(begin(), end()); }

          // Returns maximum value for the elements of this.
        inline double max() const { return *std::max_element(begin(), end()); }

          // Returns {mean, standard deviation} for the elements of this (http://en.wikipedia.org/wiki/Standard_deviation)
        inline std::pair<double, double> mean_and_standard_deviation() const
            {
                const double m = mean();
                const double sum_of_squares = std::inner_product(begin(), end(), begin(), 0.0, std::plus<double>(),
                                                                 [&m](double xx, double yy) { return (xx - m) * (yy - m); });
                return {m, std::sqrt(sum_of_squares / double(size()))};
            }

        inline double standard_deviation() const { return mean_and_standard_deviation().second; }

          // Returns distance between this and another vector
        inline double distance(const Vector& aNother) const
            {
                double dist = 0;
                auto square = [](double v) { return v * v; };
                for (const_iterator a = begin(), b = aNother.begin(); a != end(); ++a, ++b)
                    dist += square(*a - *b);
                return std::sqrt(dist);
            }

    }; // class Vector

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
