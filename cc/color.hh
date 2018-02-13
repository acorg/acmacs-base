#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

#include "acmacs-base/config.hh"
#include "acmacs-base/to-string.hh"

#ifdef ACMACS_TARGET_BROWSER
#error Not supported in cheerp client! include client/color.hh
#endif

// ----------------------------------------------------------------------

class Color
{
 public:
    using value_type = uint32_t;
    constexpr static const value_type NoChange = 0xFFFFFFFE;

    Color() : mColor(0xFF00FF) {}
    template <typename Uint, typename std::enable_if<std::is_integral<Uint>::value>::type* = nullptr> constexpr Color(Uint aColor) : mColor(static_cast<uint32_t>(aColor)) {}
    explicit Color(const std::string& aColor) { from_string(aColor); }
    explicit Color(const std::string_view& aColor) { from_string(aColor); }
    Color& operator=(const std::string& aColor) { from_string(aColor); return *this; }
    Color& operator=(const std::string_view& aColor) { from_string(aColor); return *this; }
    Color& operator=(const char* aColor) { from_string(aColor); return *this; }
    Color(const char* aColor) { from_string(aColor); }
      // Color(const Color&) = default;
      // Color& operator=(const Color& aSrc) = default;
    template <typename Uint, typename std::enable_if<std::is_integral<Uint>::value>::type* = nullptr> Color& operator=(Uint aColor) { mColor = static_cast<uint32_t>(aColor); return *this; }

    bool operator == (const Color& aColor) const { return mColor == aColor.mColor; }
    bool operator != (const Color& aColor) const { return ! operator==(aColor); }
    bool operator < (const Color& aColor) const { return mColor < aColor.mColor; }

    constexpr double alpha() const { return double(0xFF - ((mColor >> 24) & 0xFF)) / 255.0; }
    constexpr double red() const { return double((mColor >> 16) & 0xFF) / 255.0; }
    constexpr double green() const { return double((mColor >> 8) & 0xFF) / 255.0; }
    constexpr double blue() const { return double(mColor & 0xFF) / 255.0; }

    constexpr size_t alphaI() const { return static_cast<size_t>((mColor >> 24) & 0xFF); }
    constexpr void alphaI(value_type v) { mColor = (mColor & 0xFFFFFF) | ((v & 0xFF) << 24); }
    constexpr size_t rgbI() const { return static_cast<size_t>(mColor & 0xFFFFFF); }

    constexpr bool empty() const { return mColor == NoChange; }

    void light(double value);

    constexpr void set_transparency(double aTransparency) { mColor = (mColor & 0x00FFFFFF) | ((static_cast<unsigned>(aTransparency * 255.0) & 0xFF) << 24); } // for importing from lispmds
    Color without_transparency() const { return {mColor & 0x00FFFFFF}; }

    void from_string(const std::string_view& aColor);
    explicit operator std::string() const { return to_string(); }
    std::string to_string() const;
    std::string to_hex_string() const;
    void from_string(const char* s, size_t len) { from_string(std::string(s, len)); }

    enum class distinct_t { Ana, GoogleMaps };
    static std::vector<std::string> distinct_s(distinct_t dtype = distinct_t::Ana);
    static std::vector<Color> distinct(distinct_t dtype = distinct_t::Ana);

 private:
    value_type mColor; // 4 bytes, most->least significant: transparency-red-green-blue, 0x00FF0000 - opaque red, 0xFF000000 - fully transparent

}; // class Color

// ----------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& out, Color c)
{
    return out << c.to_string();
}

// ----------------------------------------------------------------------

// the functions below combined with Color imlicit constructor from string lead to incorrect conversion to Color from string-like values (e.g. Titer)
// inline std::string operator+(std::string s, Color c) { return s + c.to_string(); }
// inline std::string operator+(Color c, std::string s) { return c.to_string() + s; }
// inline std::string operator+(const char* s, Color c) { return s + c.to_string(); }
// inline std::string operator+(Color c, const char* s) { return c.to_string() + s; }

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

namespace acmacs
{
    inline std::string to_string(Color aColor)
    {
        return aColor.to_string();
    }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
