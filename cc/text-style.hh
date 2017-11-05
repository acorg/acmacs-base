#pragma once

#include <string>

#include "acmacs-base/throw.hh"
#include "acmacs-base/field.hh"
#include "acmacs-base/size.hh"
#include "acmacs-base/color.hh"

// ----------------------------------------------------------------------

namespace acmacs
{

    class TextStyle
    {
     public:
        enum class Slant { Normal, Italic };
        enum class Weight { Normal, Bold };

        inline TextStyle() : mSlant(Slant::Normal), mWeight(Weight::Normal) {}
        inline TextStyle(std::string aFontFamily, Slant aSlant = Slant::Normal, Weight aWeight = Weight::Normal) : mFontFamily(aFontFamily), mSlant(aSlant), mWeight(aWeight) {}
        inline TextStyle(std::string aFontFamily, std::string aSlant, std::string aWeight) : mFontFamily(aFontFamily) { slant(aSlant); weight(aWeight); }

        inline std::string font_family() const { return mFontFamily; }
        inline Slant slant() const { return mSlant; }
        inline Weight weight() const { return mWeight; }

        inline std::string slant_as_stirng() const
            {
                switch (mSlant) {
                  case Slant::Normal:
                      return "normal";
                  case Slant::Italic:
                      return "italic";
                }
                return "normal";
            }

        inline std::string weight_as_stirng() const
            {
                switch (mWeight) {
                  case Weight::Normal:
                      return "normal";
                  case Weight::Bold:
                      return "bold";
                }
                return "normal";
            }

        inline std::string& font_family() { return mFontFamily; }
        inline TextStyle& font_family(std::string s) { mFontFamily = s; return *this; }
        inline void font_family(const char* s, size_t len) { mFontFamily.assign(s, len); }

        inline TextStyle& slant(Slant aSlant) { mSlant = aSlant; return *this; }
        inline TextStyle& slant(std::string aSlant)
            {
                if (aSlant == "normal")
                    mSlant = Slant::Normal;
                else if (aSlant == "italic")
                    mSlant = Slant::Italic;
                else
                    THROW_OR_CERR(std::runtime_error("Unrecognized TextStyle slant: " + aSlant));
                return *this;
            }
        inline void slant(const char* s, size_t len) { slant(std::string(s, len)); }

        inline TextStyle& weight(Weight aWeight) { mWeight = aWeight; return *this; }
        inline TextStyle& weight(std::string aWeight)
            {
                if (aWeight == "normal")
                    mWeight = Weight::Normal;
                else if (aWeight == "bold")
                    mWeight = Weight::Bold;
                else
                    THROW_OR_CERR(std::runtime_error("Unrecognized TextStyle weight: " + aWeight));
                return *this;
            }
        inline void weight(const char* s, size_t len) { weight(std::string(s, len)); }

     private:
        std::string mFontFamily;
        Slant mSlant;
        Weight mWeight;

    }; // class TextStyle

// ----------------------------------------------------------------------

    class LabelStyle
    {
     public:
        inline LabelStyle() = default;

        template <typename T> using field = acmacs::internal::field_optional_with_default<T>;

        field<bool> shown{true};
        field<const acmacs::Offset&> offset{sOffsetDefault};

        // inline bool shown() const { return mShown; }
        // inline acmacs::Offset offset() const { return mOffset; }
        inline const TextStyle& text_style() const { return mStyle; }
        inline double size() const { return mSize; }
        inline Color color() const { return mColor; }
        inline Rotation rotation() const { return mRotation; }
        inline double interline() const { return mInterline; }

        // inline LabelStyle& shown(bool aShown) { mShown = aShown; return *this; }
        // inline LabelStyle& offset(const acmacs::Offset& aOffset) { mOffset = aOffset; return *this; }
        inline LabelStyle& text_style(const TextStyle& aTextStyle) { mStyle = aTextStyle; return *this; }
        inline LabelStyle& size(double aSize) { mSize = aSize; return *this; }
        inline LabelStyle& color(Color aColor) { mColor = aColor; return *this; }
        inline LabelStyle& rotation(Rotation aRotation) { mRotation = aRotation; return *this; }
        inline LabelStyle& interline(double aInterline) { mInterline = aInterline; return *this; }

     private:
        static const Offset sOffsetDefault;
        // bool mShown{true};
        // acmacs::Offset mOffset;
        TextStyle mStyle;
        double mSize{1.0};
        Color mColor{"black"};
        Rotation mRotation{NoRotation};
        double mInterline{0.2};

    }; // class LabelStyle

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
