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
    inline Color& operator=(const char* aColor) { from_string(aColor); return *this; }
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

    inline void set_transparency(double aTransparency) { mColor = (mColor & 0x00FFFFFF) | ((static_cast<unsigned>(aTransparency * 255.0) & 0xFF) << 24); } // for importing from lispmds
    inline Color without_transparency() const { return {mColor & 0x00FFFFFF}; }

    void from_string(std::string aColor);
    inline operator std::string() const { return to_string(); }
    std::string to_string() const;
    std::string to_hex_string() const;
    inline void from_string(const char* s, size_t len) { from_string(std::string(s, len)); }

    static std::vector<std::string> distinct_s();
    static std::vector<Color> distinct();

 private:
    value_type mColor; // 4 bytes, most->least significant: transparency-red-green-blue, 0x00FF0000 - opaque red, 0xFF000000 - fully transparent

}; // class Color

// ----------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& out, Color c)
{
    return out << c.to_string();
}

// ----------------------------------------------------------------------

inline std::string operator+(std::string s, Color c) { return s + c.to_string(); }
inline std::string operator+(Color c, std::string s) { return c.to_string() + s; }
inline std::string operator+(const char* s, Color c) { return s + c.to_string(); }
inline std::string operator+(Color c, const char* s) { return c.to_string() + s; }

// ----------------------------------------------------------------------

constexpr const Color ColorNoChange{Color::NoChange};
constexpr const Color BLACK{0};
constexpr const Color TRANSPARENT{0xFF000000};
constexpr const Color RED{0xFF0000};
constexpr const Color GREEN{0x00FF00};
constexpr const Color BLUE{0x0000FF};
constexpr const Color CYAN{0x00FFFF};
constexpr const Color MAGENTA{0xFF00FF};
constexpr const Color YELLOW{0xFFFF00};
constexpr const Color GREY{0xBEBEBE};
constexpr const Color GREY50{0x7F7F7F};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
