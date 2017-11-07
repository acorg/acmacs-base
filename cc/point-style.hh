#pragma once

#include "acmacs-base/text-style.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    class PointShape
    {
     public:
        enum Shape {Circle, Box, Triangle};

        inline PointShape() : mShape{Circle} {}
        inline PointShape(const PointShape&) = default;
        inline PointShape(Shape aShape) : mShape{aShape} {}
        inline PointShape(std::string aShape) { from(aShape); }
        inline PointShape& operator=(const PointShape&) = default;
        inline PointShape& operator=(Shape aShape) { mShape = aShape; return *this; }
        inline PointShape& operator=(std::string aShape) { from(aShape); return *this; }
        [[nodiscard]] inline bool operator==(const PointShape& ps) const { return mShape == ps.mShape; }

        inline operator std::string() const
            {
                switch(mShape) {
                  case Circle:
                      return "CIRCLE";
                  case Box:
                      return "BOX";
                  case Triangle:
                      return "TRIANGLE";
                }
                  //return "?";
            }

     private:
        Shape mShape;

        inline void from(std::string aShape)
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
                          std::runtime_error("Unrecognized point shape: " + aShape);
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

        [[nodiscard]] inline bool operator==(const PointStyle& ps) const
            {
                return shown == ps.shown && fill == ps.fill && outline == ps.outline && outline_width == ps.outline_width
                        && size == ps.size && rotation == ps.rotation && aspect == ps.aspect && shape == ps.shape
                        && label == ps.label && label_text == ps.label_text;
            }

        field<bool> shown{true};
        field<Color> fill{TRANSPARENT};
        field<Color> outline{BLACK};
        field<Pixels> outline_width{Pixels{1.0}};
        field<double> size{1.0};
        field<Rotation> rotation{NoRotation};
        field<Aspect> aspect{AspectNormal};
        field<PointShape> shape;
        LabelStyle label;
        field<std::string> label_text;

    }; // class PointStyle

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
