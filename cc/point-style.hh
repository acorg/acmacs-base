#pragma once

#include "acmacs-base/text-style.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    class PointShape
    {
     public:
        enum Shape {Circle, Box, Triangle, Egg, UglyEgg};

        PointShape() noexcept : mShape{Circle} {}
        PointShape(const PointShape&) noexcept = default;
        PointShape(Shape aShape) noexcept : mShape{aShape} {}
        PointShape(std::string_view aShape) { from(aShape); }
        PointShape(const char* aShape) { from(aShape); }
        PointShape& operator=(const PointShape&) noexcept = default;
        PointShape& operator=(Shape aShape) noexcept { mShape = aShape; return *this; }
        PointShape& operator=(std::string_view aShape) { from(aShape); return *this; }
        [[nodiscard]] bool operator==(const PointShape& ps) const noexcept { return mShape == ps.mShape; }
        [[nodiscard]] bool operator!=(const PointShape& ps) const noexcept { return mShape != ps.mShape; }

        constexpr operator Shape() const noexcept { return mShape; }
        constexpr Shape get() const noexcept { return mShape; }

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
                      case 'E':
                      case 'e':
                          mShape = Egg;
                          break;
                      case 'U':
                      case 'u':
                          mShape = UglyEgg;
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
                          std::runtime_error(fmt::format("Unrecognized point shape: {}", aShape));
                    }
                }
                else {
                    std::runtime_error("Unrecognized empty point shape");
                }
            }

    }; // class PointShape

    // inline std::string to_string(const PointShape& shape) { return shape; }

    // template <> inline std::string to_string(const acmacs::detail::field_optional_with_default<PointShape>& shape)
    // {
    //     if (shape.is_default())
    //         return acmacs::to_string(*shape) + "(default)";
    //     else
    //         return acmacs::to_string(*shape);
    // }

    // ----------------------------------------------------------------------

    class PointStyle
    {
      public:
        [[nodiscard]] bool operator==(const PointStyle& rhs) const noexcept
        {
            return shown == rhs.shown && fill == rhs.fill && outline == rhs.outline && outline_width == rhs.outline_width && size == rhs.size && rotation == rhs.rotation && aspect == rhs.aspect &&
                   shape == rhs.shape && label == rhs.label && label_text == rhs.label_text;
        }
        [[nodiscard]] bool operator!=(const PointStyle& rhs) const noexcept { return !operator==(rhs); }

        bool shown{true};
        Color fill{TRANSPARENT};
        Color outline{BLACK};
        Pixels outline_width{1.0};
        Pixels size{5.0};
        Rotation rotation{NoRotation};
        Aspect aspect{AspectNormal};
        PointShape shape;
        LabelStyle label;
        std::string label_text;

        PointStyle& scale(double aScale) { size = static_cast<Pixels>(size) * aScale; return *this; }
        PointStyle& scale_outline(double aScale) { outline_width = static_cast<Pixels>(outline_width) * aScale; return *this; }

    }; // class PointStyle

    // ----------------------------------------------------------------------

    struct PointStylesCompacted
    {
        std::vector<PointStyle> styles;
        std::vector<size_t> index;

    }; // class PointStylesCompacted

    class PointStyles
    {
     public:
        constexpr PointStyles() noexcept = default;
        constexpr PointStyles(const PointStyles&) noexcept = delete;
        virtual ~PointStyles() = default;

        virtual size_t number_of_points() const = 0;
        virtual bool empty() const = 0;
        virtual PointStyle style(size_t aPointNo) const = 0;
        virtual PointStylesCompacted compacted() const = 0;

    }; // class PointStyles

} // namespace acmacs

// ----------------------------------------------------------------------

template <> struct fmt::formatter<acmacs::PointShape> : fmt::formatter<acmacs::fmt_default_formatter> {
    template <typename FormatCtx> auto format(const acmacs::PointShape& shape, FormatCtx& ctx)
    {
        switch (shape.get()) {
            case acmacs::PointShape::Circle:
                return format_to(ctx.out(), "CIRCLE");
            case acmacs::PointShape::Box:
                return format_to(ctx.out(), "BOX");
            case acmacs::PointShape::Triangle:
                return format_to(ctx.out(), "TRIANGLE");
            case acmacs::PointShape::Egg:
                return format_to(ctx.out(), "EGG");
            case acmacs::PointShape::UglyEgg:
                return format_to(ctx.out(), "UGLYEGG");
        }
        return format_to(ctx.out(), "CIRCLE");
    }
};

template <> struct fmt::formatter<acmacs::PointStyle> : fmt::formatter<acmacs::fmt_default_formatter> {
    template <typename FormatCtx> auto format(const acmacs::PointStyle& style, FormatCtx& ctx) {
        return format_to(ctx.out(), R"({{"shape": {}, "shown": {}, "fill": "{}", "outline": "{}", "outline_width": {}, "size": {}, "aspect": {}, "rotation": {}, "label": {}, "label_text": "{}"}})",
                         style.shape, style.shown, style.fill, style.outline, style.outline_width, style.size, style.aspect, style.rotation, style.label, style.label_text);
    }
};


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
