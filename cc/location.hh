#pragma once

#include <array>
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

#include "acmacs-base/float.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    template <size_t N> struct Location : public std::array<double, N>
    {
        using Array = std::array<double, N>;
        using Array::operator[];
        using Array::begin;
        using Array::end;

        Location() { Array::fill(0.0); }
        Location(const Location<N>&) = default;
        Location(std::initializer_list<double> src) { assert(src.size() == Array::size()); std::copy(src.begin(), src.end(), Array::begin()); }
        Location(const std::vector<double>& aCoord) { assert(aCoord.size() == Array::size()); std::copy(aCoord.begin(), aCoord.end(), Array::begin()); }

        Location<N>& operator=(const Location<N>&) = default;
        Location<N>& operator=(const std::vector<double>& aCoord) { std::copy(aCoord.begin(), aCoord.end(), Array::begin()); }

        constexpr double x() const { return operator[](0); }
        constexpr double y() const { return operator[](1); }
        constexpr double z() const { return operator[](2); }

        void x(double val) { operator[](0) = val; }
        void y(double val) { operator[](1) = val; }
        void z(double val) { operator[](2) = val; }

        Location operator-() const;

        Location& operator += (Location loc)
            {
                std::transform(begin(), end(), loc.begin(), begin(), [](double v1, double v2) { return v1 + v2; });
                return *this;
            }

    }; // struct Location

    using Location2D = Location<2>;
    using Location3D = Location<3>;

    inline std::string to_string(Location2D loc) { return '{' + to_string(loc[0]) + ", " + to_string(loc[1]) + '}'; }
    inline std::string to_string(const Location3D& loc) { return '{' + to_string(loc[0]) + ", " + to_string(loc[1]) + ", " + to_string(loc[2]) + '}'; }
    template <size_t N> inline std::ostream& operator<<(std::ostream& out, const Location<N>& loc) { return out << to_string(loc); }

    inline double distance(Location2D a, Location2D b)
    {
        const auto dx = a.x() - b.x(), dy = a.y() - b.y();
        return std::sqrt(dx * dx + dy * dy);
    }

    template <> inline Location<2> Location<2>::operator-() const { return {-x(), -y()}; }
    template <> inline Location<3> Location<3>::operator-() const { return {-x(), -y(), -z()}; }

    inline Location2D operator+(Location2D a, Location2D b) { return {a.x() + b.x(), a.y() + b.y()}; }
    inline Location2D operator+(Location2D a, double b) { return {a.x() + b, a.y() + b}; }
    inline Location2D operator-(Location2D a, Location2D b) { return {a.x() - b.x(), a.y() - b.y()}; }
    inline Location2D operator-(Location2D a, double b) { return {a.x() - b, a.y() - b}; }
    inline Location2D operator*(Location2D a, double v) { return {a.x() * v, a.y() * v}; }
    inline Location2D operator/(Location2D a, double v) { return {a.x() / v, a.y() / v}; }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
