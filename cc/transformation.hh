#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <array>
#include <algorithm>

#include "acmacs-base/string.hh"
#include "acmacs-base/location.hh"
#include "acmacs-base/line.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    namespace detail
    {
        using TransformationBase = std::array<double, 16>;
        constexpr size_t transformation_size = 4;
        constexpr std::array<size_t, transformation_size> transformation_row_base{0, transformation_size, transformation_size * 2, transformation_size * 3};
    }

      // handles transformation and translation in 2D and 3D
    class Transformation : public detail::TransformationBase
    {
     public:
        Transformation(size_t num_dim = 2) : detail::TransformationBase{1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0}, number_of_dimensions{num_dim} {}
        Transformation(const Transformation&) = default;
        Transformation(double a11, double a12, double a21, double a22) : detail::TransformationBase{a11, a12, 0.0, 0.0, a21, a22, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0}, number_of_dimensions{2} {}
        Transformation& operator=(const Transformation&) = default;
        Transformation& operator=(Transformation&&) = default;
        bool operator==(const Transformation& rhs) const { return std::equal(begin(), end(), rhs.begin()); }
        bool operator!=(const Transformation& rhs) const { return ! operator==(rhs); }

        void reset() { operator=(Transformation()); }

        constexpr double _x(size_t offset) const { return operator[](offset); }
        constexpr double& _x(size_t offset) { return operator[](offset); }
        constexpr double _x(size_t row, size_t column) const { return operator[](detail::transformation_row_base[row] + column); }
        constexpr double& _x(size_t row, size_t column) { return operator[](detail::transformation_row_base[row] + column); }

        std::vector<double> as_vector() const
            {
                switch (number_of_dimensions) {
                  case 2:
                      return {_x(0, 0), _x(0, 1), _x(1, 0), _x(1, 1)};
                  case 3:
                      return {_x(0, 0), _x(0, 1), _x(0, 2), _x(1, 0), _x(1, 1), _x(1, 2), _x(2, 0), _x(2, 1), _x(2, 2)};
                }
                return {};
            }

          // 2D --------------------------------------------------

        constexpr double  a() const { return _x(0, 0); }
        constexpr double& a() {       return _x(0, 0); }
        constexpr double  b() const { return _x(0, 1); }
        constexpr double& b() {       return _x(0, 1); }
        constexpr double  c() const { return _x(1, 0); }
        constexpr double& c() {       return _x(1, 0); }
        constexpr double  d() const { return _x(1, 1); }
        constexpr double& d() {       return _x(1, 1); }

        Transformation& set(double a11, double a12, double a21, double a22) { a() = a11; b() = a12; c() = a21; d() = a22; return *this; }

        void rotate(double aAngle)
            {
                const double cos = std::cos(aAngle);
                const double sin = std::sin(aAngle);
                const double r0 = cos * a() + -sin * c();
                const double r1 = cos * b() + -sin * d();
                c() = sin * a() +  cos * c();
                d() = sin * b() +  cos * d();
                a() = r0;
                b() = r1;
            }

        void flip_transformed(double x, double y)
            {
                const double x2y2 = x * x - y * y, xy = 2 * x * y;
                const double r0 = x2y2 * a() + xy * c();
                const double r1 = x2y2 * b() + xy * d();
                c() = xy * a() + -x2y2 * c();
                d() = xy * b() + -x2y2 * d();
                a() = r0;
                b() = r1;
            }

          // reflect about a line specified with vector [aX, aY]
        void flip(double aX, double aY)
            {
                  // vector [aX, aY] must be first transformed using inversion of this
                const auto inv = inverse();
                const double x = aX * inv.a() + aY * inv.c();
                const double y = aX * inv.b() + aY * inv.d();
                flip_transformed(x, y);
            }

        void multiply_by(const Transformation& t)
            {
                const auto r0 = a() * t.a() + b() * t.c();
                const auto r1 = a() * t.b() + b() * t.d();
                const auto r2 = c() * t.a() + d() * t.c();
                const auto r3 = c() * t.b() + d() * t.d();
                a() = r0;
                b() = r1;
                c() = r2;
                d() = r3;
            }

        Location2D transform(Location2D loc) const
            {
                return {loc.x() * a() + loc.y() * c(), loc.x() * b() + loc.y() * d()};
            }

        LineDefinedByEquation transform(LineDefinedByEquation source) const
            {
                const auto p1 = transform(Location2D{0, source.intercept()});
                const auto p2 = transform(Location2D{1, source.slope() + source.intercept()});
                const auto slope = (p2.y() - p1.y()) / (p2.x() - p1.x());
                return {slope, p1.y() - slope * p1.x()};
            }

        double determinant() const
            {
                return a() * d() - b() * c();
            }

        class singular : public std::exception {};

        Transformation inverse() const
            {
                const auto deter = determinant();
                if (float_zero(deter))
                    throw singular{};
                return {d() / deter, - b() / deter, -c() / deter, a() / deter};
            }

        size_t number_of_dimensions = 2;

    }; // class Transformation

    inline std::string to_string(const Transformation& t)
    {
        switch (t.number_of_dimensions) {
          case 2:
              return ::string::concat("[[", t.a(), ", ", t.b(), "], [", t.c(), ", ", t.d(), "]]");
          case 3:
              return ::string::concat("[[", t._x(0, 0), t._x(0, 1), t._x(0, 2), "], [", t._x(1, 0), t._x(1, 1), t._x(1, 2), "], [", t._x(2, 0), t._x(2, 1), t._x(2, 2), "]]");
        }
        return "[[*invalid number_of_dimensions*]]";
    }

    inline std::ostream& operator << (std::ostream& out, const Transformation& t) { return out << to_string(t); }

// ----------------------------------------------------------------------

      // (N+1)xN matrix handling transformation in N-dimensional space. The last row is for translation
    class TransformationTranslation : public std::vector<double>
    {
     public:
        TransformationTranslation(size_t number_of_dimensions) : std::vector<double>((number_of_dimensions + 1) * number_of_dimensions, 0.0), number_of_dimensions_{number_of_dimensions}
            {
                for (size_t dim = 0; dim < number_of_dimensions; ++dim)
                    operator()(dim, dim) = 1.0;
            }

        TransformationTranslation(const TransformationTranslation&) = default;
        size_t number_of_dimensions() const { return number_of_dimensions_; }

        template <typename S> double& operator()(S row, S column) { return operator[](static_cast<size_t>(row) * number_of_dimensions_ + static_cast<size_t>(column)); }
        template <typename S> double operator()(S row, S column) const { return operator[](static_cast<size_t>(row) * number_of_dimensions_ + static_cast<size_t>(column)); }
        template <typename S> double& translation(S dimension) { return operator[](number_of_dimensions_ * number_of_dimensions_ + static_cast<size_t>(dimension)); }
        template <typename S> double translation(S dimension) const { return operator[](number_of_dimensions_ * number_of_dimensions_ + static_cast<size_t>(dimension)); }
        Transformation transformation() const { return {operator[](0), operator[](1), operator[](2), operator[](3)}; }

     private:
        size_t number_of_dimensions_;

    }; // class TransformationTranslation

    inline std::string to_string(const TransformationTranslation& t)
    {
        std::string result{'['};
        auto p = t.cbegin();
        for (size_t row = 0; row < (t.number_of_dimensions() + 1); ++row) {
            result += '[';
            for (size_t col = 0; col < t.number_of_dimensions(); ++col) {
                result += to_string(*p);
                if (col < t.number_of_dimensions())
                    result += ',';
                ++p;
            }
            result += "],";
        }
        result.back() = ']';
        return result;
    }

    inline std::ostream& operator << (std::ostream& out, const TransformationTranslation& t) { return out << to_string(t); }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
