#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cassert>

#include "acmacs-base/float.hh"
#include "acmacs-base/size-scale.hh"
#include "acmacs-base/transformation.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    class Size;

    class Location
    {
      public:
        double x = 0, y = 0;

        Location() = default;
        Location(const Location& src) = default;
        Location(double aX, double aY) : x(aX), y(aY) {}
        Location(Scaled aX, Scaled aY) : x(aX.value()), y(aY.value()) {}
        Location(const std::vector<double>& aCoord) : x(aCoord[0]), y(aCoord[1]) {}
        Location(const std::pair<double, double>& aCoord) : x(aCoord.first), y(aCoord.second) {}
        Location(const Size& s);

        Location& operator=(const Location& src) = default;
        Location& operator=(const std::vector<double>& aCoord) { x = aCoord[0]; y = aCoord[1]; return *this; }
        Location& operator=(std::pair<double, double> aCoord) { x = aCoord.first; y = aCoord.second; return *this; }
        void set(double aX, double aY) { x = aX; y = aY; }

        [[nodiscard]] bool operator==(const Location& loc) const { return float_equal(x, loc.x) && float_equal(y, loc.y); }
        [[nodiscard]] bool operator!=(const Location& loc) const { return !operator==(loc); }

        Location& operator-=(const Location& a)
        {
            x -= a.x;
            y -= a.y;
            return *this;
        }
        Location& operator+=(const Location& a)
        {
            x += a.x;
            y += a.y;
            return *this;
        }
        Location& operator+=(const Size& a);
        Location& operator-=(const Size& a);
        Location& operator+=(double s)
        {
            x += s;
            y += s;
            return *this;
        }
        Location& operator-=(double s)
        {
            x -= s;
            y -= s;
            return *this;
        }
        Location operator-() const { return {-x, -y}; }

        // std::string to_string() const { return "Location(" + std::to_string(x) + ", " + std::to_string(y) + ")"; }

        void min(const Location& a) noexcept
        {
            x = std::min(x, a.x);
            y = std::min(y, a.y);
        }
        void max(const Location& a) noexcept
        {
            x = std::max(x, a.x);
            y = std::max(y, a.y);
        }
        static Location center_of(const Location& a, const Location& b) { return {(a.x + b.x) / 2.0, (a.y + b.y) / 2.0}; }

        bool isnan() const noexcept { return std::isnan(x) || std::isnan(y); }
        std::vector<double> to_vector() const noexcept { return {x, y}; }

    }; // class Location

    inline std::string to_string(const Location& loc) { return '{' + to_string(loc.x) + ", " + to_string(loc.y) + '}'; }
    inline std::ostream& operator<<(std::ostream& out, const acmacs::Location& loc) { return out << to_string(loc); }

// ----------------------------------------------------------------------

    class Offset : public Location
    {
      public:
        using Location::Location;

    }; // class Offset

// ----------------------------------------------------------------------

    class Size
    {
      public:
        double width, height;

        Size() : width(0), height(0) {}
        Size(double aWidth, double aHeight) : width(aWidth), height(aHeight)
        {
            assert(width >= 0);
            assert(height >= 0);
        }
        Size(const Location& a, const Location& b) : width(std::abs(a.x - b.x)), height(std::abs(a.y - b.y))
        {
            assert(width >= 0);
            assert(height >= 0);
        }
        void set(double aWidth, double aHeight)
        {
            width = aWidth;
            height = aHeight;
            assert(width >= 0);
            assert(height >= 0);
        }
        constexpr double aspect() const noexcept { return width / height; }
        constexpr bool empty() const noexcept { return float_zero(width) && float_zero(height); }

        [[nodiscard]] bool operator==(const Size& size) const { return float_equal(width, size.width) && float_equal(height, size.height); }
        [[nodiscard]] bool operator!=(const Size& size) const { return !operator==(size); }

        // std::string to_string() const { return "Size(" + std::to_string(width) + ", " + std::to_string(height) + ")"; }

        Size& operator+=(const Size& sz)
        {
            width += sz.width;
            height += sz.height;
            return *this;
        }
        Size& operator*=(double scale)
        {
            width *= scale;
            height *= scale;
            return *this;
        }

    }; // class Size

    inline std::string to_string(const Size& size) { return '{' + to_string(size.width) + ", " + to_string(size.height) + '}'; }
    inline std::ostream& operator<<(std::ostream& out, const acmacs::Size& size) { return out << to_string(size); }

// ----------------------------------------------------------------------

    inline Location::Location(const Size& s) : x(s.width), y(s.height) {}

    inline Location& Location::operator+=(const Size& a)
    {
        x += a.width;
        y += a.height;
        return *this;
    }

    inline Location& Location::operator-=(const Size& a)
    {
        x -= a.width;
        y -= a.height;
        return *this;
    }

    inline Location operator+(const Location& a, const Size& s) { return {a.x + s.width, a.y + s.height}; }

    inline Location operator+(const Location& a, const Location& b) { return {a.x + b.x, a.y + b.y}; }

    inline Location operator-(const Location& a, const Size& s) { return {a.x - s.width, a.y - s.height}; }

    inline double distance(const Location& a, const Location& b)
    {
        const double dx = a.x - b.x, dy = a.y - b.y;
        return sqrt(dx * dx + dy * dy);
    }

    inline Location operator*(const Location& a, double v) { return {a.x * v, a.y * v}; }

    inline Location operator/(const Location& a, double v) { return {a.x / v, a.y / v}; }

    inline Size operator-(const Location& a, const Location& b) { return {a.x - b.x, a.y - b.y}; }

    inline Size operator-(const Size& a, const Location& b) { return {a.width - b.x, a.height - b.y}; }

    inline Size operator-(const Size& a, const Size& b) { return {a.width - b.width, a.height - b.height}; }

    inline Size operator+(const Size& a, const Size& b) { return {a.width + b.width, a.height + b.height}; }

    inline Size operator*(const Size& a, double v) { return {a.width * v, a.height * v}; }

    inline Size operator/(const Size& a, double v) { return {a.width / v, a.height / v}; }

// ----------------------------------------------------------------------

    class Rectangle
    {
      public:
        Rectangle(double x1, double y1, double x2, double y2) : top_left(std::min(x1, x2), std::min(y1, y2)), bottom_right(std::max(x1, x2), std::max(y1, y2)) {}

        Rectangle transform(const Transformation& aTransformation) const
        {
            const auto[x1, y1] = aTransformation.transform(top_left.x, top_left.y);
            const auto[x2, y2] = aTransformation.transform(bottom_right.x, bottom_right.y);
            return {x1, y1, x2, y2};
        }

        // returns if passed point is within the rectangle
        constexpr bool within(double x, double y) const { return x >= top_left.x && x <= bottom_right.x && y >= top_left.y && y <= bottom_right.y; }

        Location top_left;
        Location bottom_right;

    }; // class Rectangle

// ----------------------------------------------------------------------

    class Circle
    {
      public:
        Circle(double x, double y, double aRadius) : center{x, y}, radius{aRadius} {}

        Circle transform(const Transformation& aTransformation) const
        {
            const auto[x1, y1] = aTransformation.transform(center.x, center.y);
            return {x1, y1, radius};
        }

        // returns if passed point is within the circle
        bool within(double x, double y) const { return distance(center, {x, y}) <= radius; }

        Location center;
        double radius;

    }; // class Circle

// ----------------------------------------------------------------------


} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
