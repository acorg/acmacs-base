#pragma once

#include "acmacs-base/text-style.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    class PointShape
    {
     public:
        enum Shape {Circle, Box, Triangle};

        PointShape() : mShape{Circle} {}
        PointShape(const PointShape&) = default;
        PointShape(Shape aShape) : mShape{aShape} {}
        PointShape(std::string aShape) { from(aShape); }
        PointShape(std::string_view aShape) { from(aShape); }
        PointShape& operator=(const PointShape&) = default;
        PointShape& operator=(Shape aShape) { mShape = aShape; return *this; }
        PointShape& operator=(std::string aShape) { from(aShape); return *this; }
        [[nodiscard]] bool operator==(const PointShape& ps) const { return mShape == ps.mShape; }
        [[nodiscard]] bool operator!=(const PointShape& ps) const { return mShape != ps.mShape; }

        operator Shape() const { return mShape; }

        operator std::string() const
            {
                switch(mShape) {
                  case Circle:
                      return "CIRCLE";
                  case Box:
                      return "BOX";
                  case Triangle:
                      return "TRIANGLE";
                }
                return "CIRCLE"; // gcc 7.2 wants this
            }

     private:
        Shape mShape;

        void from(std::string_view aShape)
            {
                if (!aShape.empty()) {
                    switch (aShape.front()) {
                      case 'C':
                      case 'c':
                          mShape = Circle;
                          break;
                      case 'B':
                      case 'b':
                      case 'R': // rectangle
                      case 'r':
                          mShape = Box;
                          break;
                      case 'T':
                      case 't':
                          mShape = Triangle;
                          break;
                      default:
                          std::runtime_error("Unrecognized point shape: " + std::string(aShape));
                    }
                }
                else {
                    std::runtime_error("Unrecognized empty point shape");
                }
            }

    }; // class PointShape

// ----------------------------------------------------------------------

    class PointStyle
    {
      public:
        template <typename T> using field = acmacs::internal::field_optional_with_default<T>;

        [[nodiscard]] bool operator==(const PointStyle& rhs) const
        {
            return shown == rhs.shown && fill == rhs.fill && outline == rhs.outline && outline_width == rhs.outline_width && size == rhs.size && rotation == rhs.rotation && aspect == rhs.aspect &&
                   shape == rhs.shape && label == rhs.label && label_text == rhs.label_text;
        }
        [[nodiscard]] bool operator!=(const PointStyle& rhs) const { return !operator==(rhs); }

        field<bool> shown{true};
        field<Color> fill{TRANSPARENT};
        field<Color> outline{BLACK};
        field<Pixels> outline_width{Pixels{1.0}};
        field<Pixels> size{Pixels{5.0}};
        field<Rotation> rotation{NoRotation};
        field<Aspect> aspect{AspectNormal};
        field<PointShape> shape;
        LabelStyle label;
        field<std::string> label_text;

        PointStyle& scale(double aScale) { size = static_cast<Pixels>(size) * aScale; return *this; }
        PointStyle& scale_outline(double aScale) { outline_width = static_cast<Pixels>(outline_width) * aScale; return *this; }

    }; // class PointStyle

    inline std::string to_string(const acmacs::PointShape& shape) { return shape; }

    inline std::string to_string(const acmacs::PointStyle& style)
    {
        return to_string(*style.shape) + " shown:" + to_string(*style.shown) + " F:" + to_string(*style.fill) + " O:" + to_string(*style.outline) + " o:" + to_string(*style.outline_width) +
                " S:" + to_string(*style.size) + ' ' + to_string(*style.aspect) + ' ' + to_string(*style.rotation); // + " l:" + to_string(*style.label) + " t:" + to_string(*style.label_text);
    }

    inline std::ostream& operator<<(std::ostream& s, const acmacs::PointShape& shape) { return s << to_string(shape); }

    inline std::ostream& operator<<(std::ostream& s, const acmacs::PointStyle& style) { return s << to_string(style); }

} // namespace acmacs

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
