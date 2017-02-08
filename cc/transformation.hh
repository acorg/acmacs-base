#pragma once

#include <iostream>
#include <vector>
#include <cmath>

// ----------------------------------------------------------------------

class Transformation : public std::vector<double>
{
  public:
    inline Transformation() : std::vector<double>{1, 0, 0, 1} {}
    inline Transformation(const Transformation&) = default;
    inline Transformation(double a11, double a12, double a21, double a22) : std::vector<double>{a11, a12, a21, a22} {}
    inline Transformation& operator=(const Transformation& t) = default;

    inline void rotate(double aAngle)
        {
            const double cos = std::cos(aAngle);
            const double sin = std::sin(aAngle);
            const double r0 = cos * (*this)[0] + -sin * (*this)[2];
            const double r1 = cos * (*this)[1] + -sin * (*this)[3];
            (*this)[2]      = sin * (*this)[0] +  cos * (*this)[2];
            (*this)[3]      = sin * (*this)[1] +  cos * (*this)[3];
            (*this)[0] = r0;
            (*this)[1] = r1;
        }

      // reflect about a line specified with vector [aX, aY]
    inline void flip(double aX, double aY)
        {
              // vector [aX, aY] must be first transformed using this
            const double x = aX * (*this)[0] + aY * (*this)[2];
            const double y = aX * (*this)[1] + aY * (*this)[3];

            const double x2y2 = x * x - y * y, xy = 2 * x * y;
            const double r0 = x2y2 * (*this)[0] + xy    * (*this)[2];
            const double r1 = x2y2 * (*this)[1] + xy    * (*this)[3];
            (*this)[2]      = xy   * (*this)[0] + -x2y2 * (*this)[2];
            (*this)[3]      = xy   * (*this)[1] + -x2y2 * (*this)[3];
            (*this)[0] = r0;
            (*this)[1] = r1;
        }

    inline void multiply_by(const Transformation& t)
    {
        const auto r0 = (*this)[0] * t[0] + (*this)[1] * t[2];
        const auto r1 = (*this)[0] * t[1] + (*this)[1] * t[3];
        const auto r2 = (*this)[2] * t[0] + (*this)[3] * t[2];
        const auto r3 = (*this)[2] * t[1] + (*this)[3] * t[3];
        (*this)[0] = r0;
        (*this)[1] = r1;
        (*this)[2] = r2;
        (*this)[3] = r3;
    }
};

// ----------------------------------------------------------------------

inline std::ostream& operator << (std::ostream& out, const Transformation& t)
{
    return out << "[[" << t[0] << ", " << t[1] << "], [" << t[2] << ", " << t[3] << "]]";
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
