#pragma once

#include <iostream>
#include <cmath>

// ----------------------------------------------------------------------

namespace acmacs
{
    constexpr double sqr(double value) { return value * value; }

    class LineDefinedByEquation
    {
     public:
        LineDefinedByEquation() = default;
        LineDefinedByEquation(double slope, double intercept) : slope_{slope}, intercept_{intercept} {}

        constexpr double slope() const { return slope_; }
        constexpr double intercept() const { return intercept_; }

        double distance_to(double x, double y) const // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
            {
                return std::abs(slope() * x - y + intercept()) / std::sqrt(sqr(slope()) + 1);
            }

        template <typename V> double distance_to(const V& vect) const { return distance_to(vect[0], vect[1]); }

     private:
        double slope_ = 1;
        double intercept_ = 0;

    }; // class LineDefinedByEquation

    inline std::ostream& operator<<(std::ostream& out, const LineDefinedByEquation& line) { return out << "Line(slope:" << line.slope() << ", intercept:" << line.intercept() << ')'; }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
