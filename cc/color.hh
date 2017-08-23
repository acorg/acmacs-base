#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

#include "acmacs-base/config.hh"

#ifdef ACMACS_TARGET_BROWSER
#error Not supported in cheerp client! include client/color.hh
#endif

// ----------------------------------------------------------------------

class Color
{
 public:
    using value_type = uint32_t;
    constexpr static const value_type NoChange = 0xFFFFFFFE;

    inline Color() : mColor(0xFF00FF) {}
    template <typename Uint, typename std::enable_if<std::is_integral<Uint>::value>::type* = nullptr> constexpr inline Color(Uint aColor) : mColor(static_cast<uint32_t>(aColor)) {}
    inline Color(std::string aColor) { from_string(aColor); }
    inline Color& operator=(std::string aColor) { from_string(aColor); return *this; }
    inline Color(const char* aColor) { from_string(aColor); }
      // inline Color(const Color&) = default;
      // inline Color& operator=(const Color& aSrc) = default;
    template <typename Uint, typename std::enable_if<std::is_integral<Uint>::value>::type* = nullptr> inline Color& operator=(Uint aColor) { mColor = static_cast<uint32_t>(aColor); return *this; }

    inline bool operator == (const Color& aColor) const { return mColor == aColor.mColor; }
    inline bool operator != (const Color& aColor) const { return ! operator==(aColor); }
    inline bool operator < (const Color& aColor) const { return mColor < aColor.mColor; }

    inline double alpha() const { return double(0xFF - ((mColor >> 24) & 0xFF)) / 255.0; }
    inline double red() const { return double((mColor >> 16) & 0xFF) / 255.0; }
    inline double green() const { return double((mColor >> 8) & 0xFF) / 255.0; }
    inline double blue() const { return double(mColor & 0xFF) / 255.0; }

    inline size_t alphaI() const { return static_cast<size_t>((mColor >> 24) & 0xFF); }
    inline void alphaI(value_type v) { mColor = (mColor & 0xFFFFFF) | ((v & 0xFF) << 24); }
    inline size_t rgbI() const { return static_cast<size_t>(mColor & 0xFFFFFF); }

    inline bool empty() const { return mColor == NoChange; }

    void light(double value);

    // inline void set_transparency(double aTransparency) { mColor = (mColor & 0x00FFFFFF) | ((int(aTransparency * 255.0) & 0xFF) << 24); }

    void from_string(std::string aColor);
    inline operator std::string() const { return to_string(); }
    std::string to_string() const;
    std::string to_hex_string() const;
    inline void from_string(const char* s, size_t len) { from_string(std::string(s, len)); }

    static const value_type DistinctColors[];
    static std::vector<std::string> distinct_colors();

 private:
    value_type mColor; // 4 bytes, most->least significant: transparency-red-green-blue, 0x00FF0000 - opaque red, 0xFF000000 - fully transparent

}; // class Color

constexpr const Color ColorNoChange{Color::NoChange};

inline std::ostream& operator<<(std::ostream& out, Color c)
{
    return out << c.to_string();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
