#pragma once

#include <iostream>
#include <cmath>

#include "acmacs-base/point-coordinates.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    constexpr double sqr(double value) { return value * value; }

    class LineDefinedByEquation
    {
     public:
        LineDefinedByEquation() = default;
        LineDefinedByEquation(const LineDefinedByEquation&) = default;
        LineDefinedByEquation(double slope, double intercept) : slope_{slope}, intercept_{intercept} {}
        LineDefinedByEquation& operator=(const LineDefinedByEquation&) = default;

        constexpr double slope() const { return slope_; }
        constexpr double intercept() const { return intercept_; }

          // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
        double distance_with_direction(const PointCoordinates& point) const
            {
                return (slope() * point.x() - point.y() + intercept()) / std::sqrt(a2b2());
            }

        double distance_to(const PointCoordinates& point) const { return std::abs(distance_with_direction(point)); }

        PointCoordinates project_on(const PointCoordinates& source) const
            {
                return {(source.x() + slope() * source.y() - slope() * intercept()) / a2b2(),
                        (slope() * (source.x() + slope() * source.y()) + intercept()) / a2b2()};
            }

        PointCoordinates flip_over(const PointCoordinates& source, double scale = 1.0) const
            {
                return source + (project_on(source) - source) * (1.0 + scale);
            }

     private:
        double slope_ = 1;
        double intercept_ = 0;

        constexpr double a2b2() const { return sqr(slope()) + 1; }

    }; // class LineDefinedByEquation

    inline std::ostream& operator<<(std::ostream& out, const LineDefinedByEquation& line) { return out << "Line(slope:" << line.slope() << ", intercept:" << line.intercept() << ')'; }

// ----------------------------------------------------------------------

    class LineSide : public LineDefinedByEquation
    {
     public:
        enum class side { negative, positive };

        LineSide() = default;
        LineSide(double slope, double intercept, side a_side) : LineDefinedByEquation(slope, intercept), side_{a_side} {}
        LineSide(const LineDefinedByEquation& line, side a_side) : LineDefinedByEquation(line), side_{a_side} {}

          // if passed point is one the correct side, leaves it as is,
          // otherwise flips it to the correct side
        PointCoordinates fix(const PointCoordinates& source) const
            {
                if (const auto dist = distance_with_direction(source); (dist * side_sign()) < 0)
                    return flip_over(source);
                else
                    return source;
            }

     private:
        side side_ = side::positive;

        constexpr double side_sign() const { return side_ == side::negative ? -1.0 : 1.0; }

    }; // class LineSide

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
