#pragma once

#include <string>
#include <cassert>

#include "acmacs-base/size-scale.hh"
#include "acmacs-base/transformation.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    class Offset : public PointCoordinates
    {
      public:
        using PointCoordinates::PointCoordinates;

    }; // class Offset

// ----------------------------------------------------------------------

    class Size
    {
      public:
        double width = 0, height = 0;

        Size() = default;
        Size(double aWidth, double aHeight) : width(aWidth), height(aHeight)
        {
            assert(width >= 0);
            assert(height >= 0);
        }
        Size(const PointCoordinates& loc) : Size(loc.x(), loc.y()) {}
        Size(const PointCoordinates& a, const PointCoordinates& b) : Size(std::abs(a.x() - b.x()), std::abs(a.y() - b.y())) {}
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

        PointCoordinates as_location() const { return PointCoordinates(width, height); }

    }; // class Size

    inline std::string to_string(const Size& size) { return '{' + to_string(size.width) + ", " + to_string(size.height) + '}'; }
    inline std::ostream& operator<<(std::ostream& out, const acmacs::Size& size) { return out << to_string(size); }

    inline Size operator-(const Size& a, const PointCoordinates& b) { return {a.width - b.x(), a.height - b.y()}; }
    inline Size operator-(const Size& a, const Size& b) { return {a.width - b.width, a.height - b.height}; }
    inline Size operator+(const Size& a, const Size& b) { return {a.width + b.width, a.height + b.height}; }
    inline Size operator*(const Size& a, double v) { return {a.width * v, a.height * v}; }
    inline Size operator/(const Size& a, double v) { return {a.width / v, a.height / v}; }

    inline PointCoordinates operator+(const PointCoordinates& a, const Size& b) { return PointCoordinates(a.x() + b.width, a.y() + b.height); }
    inline PointCoordinates operator-(const PointCoordinates& a, const Size& b) { return PointCoordinates(a.x() - b.width, a.y() - b.height); }

// ----------------------------------------------------------------------

    class Rectangle
    {
      public:
        Rectangle(double x1, double y1, double x2, double y2) : top_left{std::min(x1, x2), std::min(y1, y2)}, bottom_right{std::max(x1, x2), std::max(y1, y2)} {}
        Rectangle(const PointCoordinates& a, const PointCoordinates& b) : Rectangle(a.x(), a.y(), b.x(), b.y()) {}

        Rectangle transform(const Transformation& aTransformation) const
        {
            return {aTransformation.transform(top_left), aTransformation.transform(bottom_right)};
        }

        // returns if passed point is within the rectangle
        // constexpr bool within(double x, double y) const { return x >= top_left.x() && x <= bottom_right.x() && y >= top_left.y() && y <= bottom_right.y(); }
        bool within(const PointCoordinates& loc) const { return loc.x() >= top_left.x() && loc.x() <= bottom_right.x() && loc.y() >= top_left.y() && loc.y() <= bottom_right.y(); }

        PointCoordinates top_left;
        PointCoordinates bottom_right;

    }; // class Rectangle

// ----------------------------------------------------------------------

    class Circle
    {
      public:
          // Circle(double x, double y, double aRadius) : center{x, y}, radius{aRadius} {}
        Circle(const PointCoordinates& aCenter, double aRadius) : center{aCenter}, radius{aRadius} {}

        Circle transform(const Transformation& aTransformation) const
        {
            return {aTransformation.transform(center), radius};
        }

        // returns if passed point is within the circle
        // bool within(double x, double y) const { return distance(center, {x, y}) <= radius; }
        bool within(const PointCoordinates& loc) const { return distance(center, loc) <= radius; }

        PointCoordinates center;
        double radius;

    }; // class Circle

// ----------------------------------------------------------------------


} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
