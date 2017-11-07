#pragma once

#include <string>

#include "acmacs-base/field.hh"
#include "acmacs-base/size.hh"
#include "acmacs-base/color.hh"

// ----------------------------------------------------------------------

namespace acmacs
{

    class FontSlant
    {
     public:
        enum Value { Normal, Italic };

        inline FontSlant(Value aFontSlant = Normal) : mFontSlant{aFontSlant} {}
        inline FontSlant(const FontSlant&) = default;
        inline FontSlant(std::string aFontSlant) { from(aFontSlant); }
        inline FontSlant& operator=(const FontSlant&) = default;
        inline FontSlant& operator=(std::string aFontSlant) { from(aFontSlant); return *this; }

        inline operator std::string() const
            {
                switch (mFontSlant) {
                  case Normal:
                      return "normal";
                  case Italic:
                      return "italic";
                }
                return "normal";
            }

        inline operator Value() const { return mFontSlant; }

     private:
        Value mFontSlant;

        inline void from(std::string aFontSlant)
            {
                if (aFontSlant == "normal")
                    mFontSlant = Normal;
                else if (aFontSlant == "italic")
                    mFontSlant = Italic;
                else
                    std::runtime_error("Unrecognized slant: " + aFontSlant);
            }

    }; // class FontSlant

// ----------------------------------------------------------------------

    class FontWeight
    {
     public:
        enum Value { Normal, Bold };

        inline FontWeight(Value aFontWeight = Normal) : mFontWeight{aFontWeight} {}
        inline FontWeight(const FontWeight&) = default;
        inline FontWeight(std::string aFontWeight) { from(aFontWeight); }
        inline FontWeight& operator=(const FontWeight&) = default;
        inline FontWeight& operator=(std::string aFontWeight) { from(aFontWeight); return *this; }

        inline operator std::string() const
            {
                switch (mFontWeight) {
                  case Normal:
                      return "normal";
                  case Bold:
                      return "bold";
                }
                return "normal";
            }

        inline operator Value() const { return mFontWeight; }

     private:
        Value mFontWeight;

        inline void from(std::string aFontWeight)
            {
                if (aFontWeight == "normal")
                    mFontWeight = Normal;
                else if (aFontWeight == "bold")
                    mFontWeight = Bold;
                else
                    std::runtime_error("Unrecognized weight: " + aFontWeight);
            }

    }; // class FontWeight

// ----------------------------------------------------------------------

    class TextStyle
    {
     public:
        template <typename T> using field = acmacs::internal::field_optional_with_default<T>;

        inline TextStyle() = default;
        inline TextStyle(std::string font_name) : font_family{font_name} {}

        [[nodiscard]] inline bool operator==(const TextStyle& ts) const
            {
                return slant == ts.slant && weight == ts.weight && font_family == ts.font_family;
            }

        field<FontSlant> slant;
        field<FontWeight> weight;
        field<std::string> font_family;

    }; // class TextStyle

// ----------------------------------------------------------------------

    class LabelStyle
    {
     public:
        template <typename T> using field = acmacs::internal::field_optional_with_default<T>;

        [[nodiscard]] inline bool operator==(const LabelStyle& ls) const
            {
                return shown == ls.shown && offset == ls.offset && size == ls.size && color == ls.color
                        && rotation == ls.rotation && interline == ls.interline && style == ls.style;
            }

        field<bool> shown{true};
        field<const acmacs::Offset&> offset{sOffsetDefault};
        field<double> size{1.0};
        field<Color> color{BLACK};
        field<Rotation> rotation{NoRotation};
        field<double> interline{0.2};
        TextStyle style;

     private:
        static const Offset sOffsetDefault;

    }; // class LabelStyle

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
