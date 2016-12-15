#pragma once

#include <string>
#include <cstdint>

// ----------------------------------------------------------------------

class Color
{
 public:
    using value_type = uint32_t;

    inline Color() : mColor(0xFF00FF) {}
    template <typename Uint, typename std::enable_if<std::is_integral<Uint>::value>::type* = nullptr> constexpr inline Color(Uint aColor) : mColor(static_cast<uint32_t>(aColor)) {}
    inline Color(std::string aColor) { from_string(aColor); }
    inline Color(const char* aColor) { from_string(aColor); }
      // inline Color(const Color&) = default;
      // inline Color& operator=(const Color& aSrc) = default;
    template <typename Uint, typename std::enable_if<std::is_integral<Uint>::value>::type* = nullptr> inline Color& operator=(Uint aColor) { mColor = static_cast<uint32_t>(aColor); return *this; }
    inline Color& operator=(std::string aColor) { from_string(aColor); return *this; }

    inline bool operator == (const Color& aColor) const { return mColor == aColor.mColor; }
    inline bool operator != (const Color& aColor) const { return ! operator==(aColor); }

    inline double alpha() const { return double(0xFF - ((mColor >> 24) & 0xFF)) / 255.0; }
    inline double red() const { return double((mColor >> 16) & 0xFF) / 255.0; }
    inline double green() const { return double((mColor >> 8) & 0xFF) / 255.0; }
    inline double blue() const { return double(mColor & 0xFF) / 255.0; }

    inline size_t alphaI() const { return static_cast<size_t>((mColor >> 24) & 0xFF); }
    inline void alphaI(value_type v) { mColor = (mColor & 0xFFFFFF) | ((v & 0xFF) << 24); }
    inline size_t rgbI() const { return static_cast<size_t>(mColor & 0xFFFFFF); }

    // inline void set_transparency(double aTransparency) { mColor = (mColor & 0x00FFFFFF) | ((int(aTransparency * 255.0) & 0xFF) << 24); }

    inline operator std::string() const { return to_string(); }

    std::string to_string() const;
    void from_string(std::string aColor);

      // http://stackoverflow.com/questions/470690/how-to-automatically-generate-n-distinct-colors (search for kellysMaxContrastSet)
    constexpr static const value_type DistinctColors[] = {
        0xA6BDD7, //Very Light Blue
        0xC10020, //Vivid Red
        0xFFB300, //Vivid Yellow
        0x803E75, //Strong Purple
        0xFF6800, //Vivid Orange
        0xCEA262, //Grayish Yellow
          //0x817066, //Medium Gray

          //The following will not be good for people with defective color vision
        0x007D34, //Vivid Green
        0xF6768E, //Strong Purplish Pink
        0x00538A, //Strong Blue
        0xFF7A5C, //Strong Yellowish Pink
        0x53377A, //Strong Violet
        0xFF8E00, //Vivid Orange Yellow
        0xB32851, //Strong Purplish Red
        0xF4C800, //Vivid Greenish Yellow
        0x7F180D, //Strong Reddish Brown
        0x93AA00, //Vivid Yellowish Green
        0x593315, //Deep Yellowish Brown
        0xF13A13, //Vivid Reddish Orange
        0x232C16, //Dark Olive Green
    };

 private:
    value_type mColor; // 4 bytes, most->least significant: transparency-red-green-blue, 0x00FF0000 - opaque red, 0xFF000000 - fully transparent

}; // class Color

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
