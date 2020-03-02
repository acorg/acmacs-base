#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

#include "acmacs-base/to-string.hh"
#include "acmacs-base/flat-map.hh"

// ----------------------------------------------------------------------

class Color
{
 public:
    union value_type
    {
        constexpr value_type() : color{0xFF00FF} {}
        constexpr value_type(uint32_t regular) : color{regular} {}
        constexpr value_type(double adj) : adjustment{static_cast<float>(adj)} {}
        constexpr bool operator==(const value_type& rhs) const { return color == rhs.color; }
        uint32_t color;
        float adjustment;
    };
      // constexpr static const uint32_t NoChange = 0xFFFFFFFE;
    enum class type : char { regular, no_change, adjust_saturation, adjust_brightness, adjust_transparency };

    Color() = default;
    Color(const Color& aSrc) = default;
    template <typename Uint, typename std::enable_if<std::is_integral<Uint>::value>::type* = nullptr> constexpr Color(Uint aColor) : color_(static_cast<uint32_t>(aColor)) {}
    explicit Color(const std::string& aColor) { from_string(aColor); }
    explicit Color(const std::string_view& aColor) { from_string(aColor); }
    constexpr explicit Color(type a_type, double value) : color_(value), type_(a_type) {}
    Color& operator=(const std::string& aColor) { from_string(aColor); return *this; }
    Color& operator=(const std::string_view& aColor) { from_string(aColor); return *this; }
    Color& operator=(const char* aColor) { from_string(aColor); return *this; }
    Color(const char* aColor) { from_string(aColor); }
      // Color(const Color&) = default;
    Color& operator=(const Color& aSrc);
    template <typename Uint, typename std::enable_if<std::is_integral<Uint>::value>::type* = nullptr> Color& operator=(Uint aColor)
        {
            color_ = static_cast<uint32_t>(aColor);
            type_ = type::regular;
            return *this;
        }

    bool operator == (const Color& aColor) const { return color_ == aColor.color_ && type_ == aColor.type_; }
    bool operator != (const Color& aColor) const { return ! operator==(aColor); }
    bool operator < (const Color& aColor) const { return color_.color < aColor.color_.color; }

    constexpr double alpha() const { return double(0xFF - ((color_.color >> 24) & 0xFF)) / 255.0; } // 0.0 - transparent, 1.0 - opaque
    constexpr double red() const { return double((color_.color >> 16) & 0xFF) / 255.0; }
    constexpr double green() const { return double((color_.color >> 8) & 0xFF) / 255.0; }
    constexpr double blue() const { return double(color_.color & 0xFF) / 255.0; }

    constexpr size_t alphaI() const { return static_cast<size_t>((color_.color >> 24) & 0xFF); }
    constexpr void alphaI(uint32_t v) { color_.color = (color_.color & 0xFFFFFF) | ((v & 0xFF) << 24); }
    constexpr size_t rgbI() const { return static_cast<size_t>(color_.color & 0xFFFFFF); }

    constexpr bool empty() const { return type_ == type::no_change; }

    void light(double value);
    void adjust_saturation(double value);
    void adjust_brightness(double value);
    void adjust_transparency(double value);

    constexpr void set_transparency(double aTransparency) { color_.color = (color_.color & 0x00FFFFFF) | ((static_cast<unsigned>(aTransparency * 255.0) & 0xFF) << 24); } // for importing from lispmds
    Color without_transparency() const { return {color_.color & 0x00FFFFFF}; }

    void from_string(const std::string_view& aColor);
    explicit operator std::string() const { return to_string(); }
    std::string to_string() const;
    std::string to_hex_string() const;
    void from_string(const char* s, size_t len) { from_string(std::string(s, len)); }

    enum class distinct_t { Ana, GoogleMaps };
    static std::vector<std::string> distinct_s(distinct_t dtype = distinct_t::Ana);
    static std::vector<Color> distinct(distinct_t dtype = distinct_t::Ana);
    static Color distinct(size_t offset, distinct_t dtype = distinct_t::Ana);
    static Color perceptually_uniform_heatmap(size_t total_colors, size_t color_index);

 private:
      // 4 bytes: most->least significant: transparency-red-green-blue, 0x00FF0000 - opaque red, 0xFF000000 - fully transparent
    value_type color_;
    type type_ = type::regular;

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

constexpr const Color ColorNoChange(Color::type::no_change, 0);
constexpr const Color BLACK{0};
constexpr const Color WHITE{0xFFFFFF};
constexpr const Color GREY97{0xF7F7F7};
constexpr const Color TRANSPARENT{0xFF000000};
constexpr const Color RED{0xFF0000};
constexpr const Color GREEN{0x00FF00};
constexpr const Color BLUE{0x0000FF};
constexpr const Color CYAN{0x00FFFF};
constexpr const Color MAGENTA{0xFF00FF};
constexpr const Color PINK{0xFFC0CB};
constexpr const Color YELLOW{0xFFFF00};
constexpr const Color GREY{0xBEBEBE};
constexpr const Color GREY50{0x7F7F7F};

const acmacs::flat_map_t<std::string, Color>& continent_colors();
const acmacs::flat_map_t<std::string, Color>& continent_colors_dark();
Color continent_color(std::string_view continent);
Color continent_color_dark(std::string_view continent);

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
