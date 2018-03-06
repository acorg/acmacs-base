#pragma once

#include <string>

#include "acmacs-base/size.hh"
#include "acmacs-base/color.hh"
#include "acmacs-base/field.hh"

// ----------------------------------------------------------------------

namespace acmacs
{

    class FontSlant
    {
     public:
        enum Value { Normal, Italic };

        FontSlant(Value aFontSlant = Normal) : mFontSlant{aFontSlant} {}
        FontSlant(const FontSlant&) = default;
        FontSlant(std::string aFontSlant) { from(aFontSlant); }
        FontSlant& operator=(const FontSlant&) = default;
        FontSlant& operator=(std::string aFontSlant) { from(aFontSlant); return *this; }

        operator std::string() const
            {
                switch (mFontSlant) {
                  case Normal:
                      return "normal";
                  case Italic:
                      return "italic";
                }
                return "normal";
            }

        operator Value() const { return mFontSlant; }

     private:
        Value mFontSlant;

        void from(std::string aFontSlant)
            {
                if (aFontSlant == "normal")
                    mFontSlant = Normal;
                else if (aFontSlant == "italic")
                    mFontSlant = Italic;
                else
                    std::runtime_error("Unrecognized slant: " + aFontSlant);
            }

    }; // class FontSlant

    inline std::string to_string(const FontSlant& slant) { return slant; }

// ----------------------------------------------------------------------

    class FontWeight
    {
     public:
        enum Value { Normal, Bold };

        FontWeight(Value aFontWeight = Normal) : mFontWeight{aFontWeight} {}
        FontWeight(const FontWeight&) = default;
        FontWeight(std::string aFontWeight) { from(aFontWeight); }
        FontWeight& operator=(const FontWeight&) = default;
        FontWeight& operator=(std::string aFontWeight) { from(aFontWeight); return *this; }

        operator std::string() const
            {
                switch (mFontWeight) {
                  case Normal:
                      return "normal";
                  case Bold:
                      return "bold";
                }
                return "normal";
            }

        operator Value() const { return mFontWeight; }

     private:
        Value mFontWeight;

        void from(std::string aFontWeight)
            {
                if (aFontWeight == "normal")
                    mFontWeight = Normal;
                else if (aFontWeight == "bold")
                    mFontWeight = Bold;
                else
                    std::runtime_error("Unrecognized weight: " + aFontWeight);
            }

    }; // class FontWeight

    inline std::string to_string(const FontWeight& weight) { return weight; }

// ----------------------------------------------------------------------

    class TextStyle
    {
     public:
        template <typename T> using field = acmacs::internal::field_optional_with_default<T>;

        TextStyle() = default;
        TextStyle(std::string font_name) : font_family{font_name} {}

        [[nodiscard]] bool operator==(const TextStyle& ts) const
            {
                return slant == ts.slant && weight == ts.weight && font_family == ts.font_family;
            }

        field<FontSlant> slant;
        field<FontWeight> weight;
        field<std::string> font_family;

    }; // class TextStyle

    inline std::string to_string(const TextStyle& style)
    {
        return " S:" + to_string(*style.slant) + " W:" + to_string(*style.weight) + " f:" + to_string(*style.font_family);
    }

// ----------------------------------------------------------------------

    class LabelStyle
    {
     public:
        template <typename T> using field = acmacs::internal::field_optional_with_default<T>;

        [[nodiscard]] bool operator==(const LabelStyle& ls) const
            {
                return shown == ls.shown && offset == ls.offset && size == ls.size && color == ls.color
                        && rotation == ls.rotation && interline == ls.interline && style == ls.style;
            }
        [[nodiscard]] bool operator!=(const LabelStyle& rhs) const { return !operator==(rhs); }

        field<bool> shown{true};
        field<Offset> offset{sOffsetDefault};
        field<Pixels> size{Pixels{10.0}};
        field<Color> color{BLACK};
        field<Rotation> rotation{NoRotation};
        field<double> interline{0.2};
        TextStyle style;

     private:
        static const Offset sOffsetDefault;

    }; // class LabelStyle

    inline std::string to_string(const LabelStyle& style)
    {
        return " shown:" + to_string(*style.shown) + " p:" + to_string(*style.offset) + " s:" + to_string(*style.size) +
                " c:" + to_string(*style.color) + " r:" + to_string(*style.rotation) + " i:" + acmacs::to_string(*style.interline, 2) + ' ' + to_string(style.style);
    }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
