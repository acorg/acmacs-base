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

          // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
        template <typename V> double distance_with_direction(const V& vect) const
            {
                return (slope() * vect[0] - vect[1] + intercept()) / std::sqrt(a2b2());
            }

        template <typename V> double distance_to(const V& vect) const { return std::abs(distance_with_direction(vect)); }

        template <typename V> V project_on(const V& source) const
            {
                return {(source[0] + slope() * source[1] - slope() * intercept()) / a2b2(),
                        (slope() * (source[0] + slope() * source[1]) + intercept()) / a2b2()};
            }

        template <typename V> V flip_over(const V& source, double scale = 1.0) const
            {
                return source + (project_on(source) - source) * (1.0 + scale);
            }

     private:
        double slope_ = 1;
        double intercept_ = 0;

        constexpr double a2b2() const { return sqr(slope()) + 1; }

    }; // class LineDefinedByEquation

    inline std::ostream& operator<<(std::ostream& out, const LineDefinedByEquation& line) { return out << "Line(slope:" << line.slope() << ", intercept:" << line.intercept() << ')'; }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
