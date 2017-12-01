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
        inline Vector& multiply_by(double aAlpha)
            {
                std::transform(this->begin(), this->end(), this->begin(), std::bind(std::multiplies<double>(), std::placeholders::_1, aAlpha));
                return *this;
            }

          // Add aValue to each vector element
        inline Vector& add(double aValue)
            {
                std::transform(this->begin(), this->end(), this->begin(), std::bind(std::plus<double>(), std::placeholders::_1, aValue));
                return *this;
            }

          // Adds corresponding elements of y from elements of this.
          // this = this + y
        inline Vector& add(const Vector& y)
            {
                std::transform(this->begin(), this->end(), y.begin(), this->begin(), std::plus<double>());
                return *this;
            }

          // Subtracts corresponding elements of y from elements of this (supports Vector and VectorView)
          // this' = this - y
        inline Vector& subtract(const Vector& y)
            {
                std::transform(this->begin(), this->end(), y.begin(), this->begin(), std::minus<double>());
                return *this;
            }

          // Sets all elemets of this to zero
        inline void zero()
            {
                std::fill(this->begin(), this->end(), 0.0);
            }

          // Computes the Euclidean norm ||x||_2 = \sqrt {\sum x_i^2} of this.
        inline double eucledian_norm() const
            {
                return std::sqrt(std::inner_product(this->begin(), this->end(), this->begin(), 0.0));
            }

          // Computes the scalar product x^T y for this and y.
        inline double scalar_product(const Vector& y) const
            {
                return std::inner_product(this->begin(), this->end(), y.begin(), 0.0);
            }

        inline double inner_product() const
            {
                return std::inner_product(this->begin(), this->end(), this->begin(), 0.0);
            }

          // Increments each element of y by aAlpha*this (y = alpha this + y). this unchanged.
        inline void axpy(double aAlpha, Vector& y) const
            {
                std::transform(this->begin(), this->end(), y.begin(), y.begin(), [&aAlpha](double xx, double yy) { return aAlpha * xx + yy; });
            }

          // Returns mean value for the elements of this.
        inline double mean() const
            {
                return std::accumulate(this->begin(), this->end(), 0.0) / this->size();
            }

          // Returns minimum value for the elements of this.
        inline double min() const
            {
                return *std::min_element(this->begin(), this->end());
            }

          // Returns maximum value for the elements of this.
        inline double max() const
            {
                return *std::max_element(this->begin(), this->end());
            }

          // Returns standard deviation for the elements of this (http://en.wikipedia.org/wiki/Standard_deviation)
        inline double standard_deviation() const
            {
                const double m = mean();
                const double sum_of_squares = std::inner_product(this->begin(), this->end(), this->begin(), 0.0, std::plus<double>(),
                                                                 [&m](double xx, double yy) { return (xx - m) * (yy - m); });
                return std::sqrt(sum_of_squares / double(this->size()));
            }

          // Returns distance between this and another vector
        inline double distance(const Vector& aNother) const
            {
                double dist = 0;
                auto square = [](double v) { return v * v; };
                for (const_iterator a = this->begin(), b = aNother.begin(); a != this->end(); ++a, ++b)
                    dist += square(*a - *b);
                return std::sqrt(dist);
            }

    }; // class Vector

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
