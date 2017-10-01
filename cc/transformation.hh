#pragma once

#include <iostream>
#include <cmath>

#include "acmacs-base/float.hh"

// ----------------------------------------------------------------------

class Transformation
{
 public:
    double a = 1.0, b = 0.0, c = 0.0, d = 1.0;

    inline Transformation() = default;
    inline Transformation(const Transformation&) = default;
    inline Transformation(double a11, double a12, double a21, double a22) : a{a11}, b{a12}, c{a21}, d{a22} {}
    // inline Transformation& operator=(const Transformation& t) = default;

    inline void rotate(double aAngle)
        {
            const double cos = std::cos(aAngle);
            const double sin = std::sin(aAngle);
            const double r0 = cos * a + -sin * c;
            const double r1 = cos * b + -sin * d;
            c = sin * a +  cos * c;
            d = sin * b +  cos * d;
            a = r0;
            b = r1;
        }

      // reflect about a line specified with vector [aX, aY]
    inline void flip(double aX, double aY)
        {
              // vector [aX, aY] must be first transformed using this
            const double x = aX * a + aY * c;
            const double y = aX * b + aY * d;

            const double x2y2 = x * x - y * y, xy = 2 * x * y;
            const double r0 = x2y2 * a + xy * c;
            const double r1 = x2y2 * b + xy * d;
            c = xy * a + -x2y2 * c;
            d = xy * b + -x2y2 * d;
            a = r0;
            b = r1;
        }

    inline void multiply_by(const Transformation& t)
    {
        const auto r0 = a * t.a + b * t.c;
        const auto r1 = a * t.b + b * t.d;
        const auto r2 = c * t.a + d * t.c;
        const auto r3 = c * t.b + d * t.d;
        a = r0;
        b = r1;
        c = r2;
        d = r3;
    }

    inline std::pair<double, double> transform(double x, double y) const
        {
            return std::make_pair(x * a + y * c, x * b + y * d);
        }

    inline double determinant() const
        {
            return a * d - b * c;
        }

    class singular : public std::exception {};

    inline Transformation inverse() const
        {
            const auto deter = determinant();
            if (float_zero(deter))
                throw singular{};
            return {d / deter, - b / deter, -c / deter, a / deter};
        }
};

// ----------------------------------------------------------------------

inline std::ostream& operator << (std::ostream& out, const Transformation& t)
{
    return out << "[[" << t.a << ", " << t.b << "], [" << t.c << ", " << t.d << "]]";
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
