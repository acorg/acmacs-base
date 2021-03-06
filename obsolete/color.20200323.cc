#include <map>
#include <cmath>
#include <array>

#include "acmacs-base/color.hh"
#include "acmacs-base/string.hh"

// ----------------------------------------------------------------------

namespace _internal
{
    extern const std::map<uint32_t, std::string> sColorToName;
    extern const std::map<std::string, uint32_t> sNameToColor;
    extern std::array<uint32_t, 256> sScaleColorPerceptualViridis;
}

// ----------------------------------------------------------------------

Color& Color::operator=(const Color& src)
{
    switch (type_) {
      case type::no_change:
          type_ = src.type_;
          color_ = src.color_;
          break;
      case type::regular:
          switch (src.type_) {
            case type::no_change:
                break;
            case type::regular:
                type_ = type::regular;
                color_ = src.color_;
                break;
            case type::adjust_saturation:
                adjust_saturation(static_cast<double>(src.color_.adjustment));
                break;
            case type::adjust_brightness:
                adjust_brightness(static_cast<double>(src.color_.adjustment));
                break;
            case type::adjust_transparency:
                adjust_transparency(static_cast<double>(src.color_.adjustment));
                break;
          }
          break;
      case type::adjust_saturation:
      case type::adjust_brightness:
      case type::adjust_transparency:
          throw std::runtime_error("Color::operator=: cannot assign to a non-regular color");
    }

    return *this;

} // Color::operator=

// ----------------------------------------------------------------------

std::string Color::to_string() const
{
    using namespace _internal;
    std::string result;
    switch (type_) {
      case type::no_change:
          result = "*no_change*";
          break;
      case type::regular:
          // if (const auto e = std::find_if(sNameToColor.cbegin(), sNameToColor.cend(), [this](const auto& ee) { return this->color_.color == ee.second; }); e != sNameToColor.cend())
          //     result = e->first;
          if (const auto found = sColorToName.find(color_.color); found != sColorToName.end())
              result = found->second;
          else if (alphaI() == 0)
              result = to_hex_string();
          else
              result = to_rgba_string();
          break;
      case type::adjust_saturation:
          result = "*adjust_saturation " + std::to_string(color_.adjustment) + '*';
          break;
      case type::adjust_brightness:
          result = "*adjust_brightness " + std::to_string(color_.adjustment) + '*';
          break;
      case type::adjust_transparency:
          result = "*adjust_transparency " + std::to_string(color_.adjustment) + '*';
          break;
    }
    return result;

} // Color::to_string

// ----------------------------------------------------------------------

std::string Color::to_hex_string() const
{
    return fmt::format("#{:06X}", color_.color);

} // Color::to_hex_string

// ----------------------------------------------------------------------

std::string Color::to_rgba_string() const
{
    return fmt::format("rgba({},{},{},{:.3f})", redI(), greenI(), blueI(), opacity());

} // Color::to_rgba_string

// ----------------------------------------------------------------------

void Color::from_string(const std::string_view& aColor)
{
    if (aColor.empty())
        throw std::invalid_argument("cannot read Color from empty string");
    type_ = type::regular;
    if (aColor[0] == '#') {
        const auto v = static_cast<uint32_t>(std::strtoul(aColor.data() + 1, nullptr, 16));
        switch (aColor.size()) {
          case 4:               // web color #abc -> #aabbcc
              color_.color = ((v & 0xF00) << 12) | ((v & 0xF00) << 8) | ((v & 0x0F0) << 8) | ((v & 0x0F0) << 4) | ((v & 0x00F) << 4) | (v & 0x00F);
              break;
          case 7:
          case 9:
              color_.color = v;
              break;
          default:
              throw std::invalid_argument("cannot read Color from " + acmacs::to_string(aColor));
        }
    }
    else {
        const auto e = _internal::sNameToColor.find(string::lower(aColor));
        if (e != _internal::sNameToColor.end())
            color_.color = e->second;
        else
            throw std::invalid_argument("cannot read Color from " + acmacs::to_string(aColor));
    }

} // Color::from_string

// ----------------------------------------------------------------------

struct HSV;

struct RGB {
    RGB(Color c) : r(c.red()), g(c.green()), b(c.blue()) {}
    RGB(const HSV& hsv);
    double r;       // percent
    double g;       // percent
    double b;       // percent
};

struct HSV {
    HSV(const RGB& rgb);
    void light(double value) { s /= value; }
    void adjust_saturation(double value) { s *= std::abs(value); if (s > 1.0) s = 1.0; }
    void adjust_brightness(double value) { v *= std::abs(value); if (v > 1.0) v = 1.0; }
    double h;       // angle in degrees
    double s;       // percent
    double v;       // percent
};

// http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
RGB::RGB(const HSV& hsv)
{
    if (hsv.s <= 0.0) {       // < is bogus, just shuts up warnings
        r = hsv.v;
        g = hsv.v;
        b = hsv.v;
    }
    else {
        double hh = hsv.h;
        if (hh >= 360.0)
            hh = 0.0;
        hh /= 60.0;
        double ff = hh - long(hh);
        double p = hsv.v * (1.0 - hsv.s);
        double q = hsv.v * (1.0 - (hsv.s * ff));
        double t = hsv.v * (1.0 - (hsv.s * (1.0 - ff)));

        switch(long(hh)) {
          case 0:
              r = hsv.v;
              g = t;
              b = p;
              break;
          case 1:
              r = q;
              g = hsv.v;
              b = p;
              break;
          case 2:
              r = p;
              g = hsv.v;
              b = t;
              break;
          case 3:
              r = p;
              g = q;
              b = hsv.v;
              break;
          case 4:
              r = t;
              g = p;
              b = hsv.v;
              break;
          case 5:
          default:
              r = hsv.v;
              g = p;
              b = q;
              break;
        }
    }
}

inline std::ostream& operator << (std::ostream& out, const RGB& rgb)
{
    return out << rgb.r << ':' << rgb.g << ':' << rgb.b;
}

HSV::HSV(const RGB& rgb)
{
    double min = rgb.r < rgb.g ? rgb.r : rgb.g;
    min = min  < rgb.b ? min  : rgb.b;

    double max = rgb.r > rgb.g ? rgb.r : rgb.g;
    max = max  > rgb.b ? max  : rgb.b;

    v = max;
    double delta = max - min;
    if (delta < 0.00001) {
        s = 0;
        h = 0; // undefined, maybe nan?
    }
    else if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
        s = delta / max;                  // s
        if (rgb.r >= max) {                           // > is bogus, just keeps compilor happy
            h = (rgb.g - rgb.b) / delta;        // between yellow & magenta
        }
        else {
            if (rgb.g >= max)
                h = 2.0 + (rgb.b - rgb.r) / delta;  // between cyan & yellow
            else
                h = 4.0 + (rgb.r - rgb.g) / delta;  // between magenta & cyan
        }
        h *= 60.0;                              // degrees
        if (h < 0.0)
            h += 360.0;
    }
    else {
          // if max is 0, then r = g = b = 0
          // s = 0, v is undefined
        s = 0.0;
        h = 0.0; // NAN;                            // its now undefined
    }
}

void Color::light(double value)
{
    if ((color_.color & 0xFF000000) == 0) {
        if (color_.color == 0) {
              // special case for black
            HSV hsv = RGB(0xFF0000);
            hsv.light(value);
            RGB rgb(hsv);
            color_.color = (unsigned(rgb.b * 255) << 16) | (unsigned(rgb.b * 255) << 8) | unsigned(rgb.b * 255);
        }
        else {
            HSV hsv = RGB(color_.color);
            hsv.light(value);
            RGB rgb(hsv);
              // std::cerr << std::hex << color_ << "  " << std::dec << RGB(color_) << " --> " << rgb << std::endl;
            color_.color = (unsigned(rgb.r * 255) << 16) | (unsigned(rgb.g * 255) << 8) | unsigned(rgb.b * 255);
        }
    }

} // Color::light

void Color::adjust_saturation(double value)
{
    if ((color_.color & 0xFF000000) == 0) {
        HSV hsv = RGB(color_.color);
        // std::cerr << "DEBUG: adjust_saturation " << std::hex << color_.color << std::dec << " s:" << hsv.s << " v:" << hsv.v << " --> ";
        hsv.adjust_saturation(value);
        RGB rgb(hsv);
          // std::cerr << std::hex << color_ << "  " << std::dec << RGB(color_) << " --> " << rgb << std::endl;
        color_.color = (unsigned(rgb.r * 255) << 16) | (unsigned(rgb.g * 255) << 8) | unsigned(rgb.b * 255);
        // std::cerr << std::hex << color_.color << std::dec << ' ' << hsv.s << '\n';
    }
}

void Color::adjust_brightness(double value)
{
    if ((color_.color & 0xFF000000) == 0) {
        HSV hsv = RGB(color_.color);
        hsv.adjust_brightness(value);
        RGB rgb(hsv);
          // std::cerr << std::hex << color_ << "  " << std::dec << RGB(color_) << " --> " << rgb << std::endl;
        color_.color = (unsigned(rgb.r * 255) << 16) | (unsigned(rgb.g * 255) << 8) | unsigned(rgb.b * 255);
    }
}

void Color::adjust_transparency(double value)
{
    if (value < 0.0)
        throw std::invalid_argument(fmt::format("Color::adjust_transparency: attempt to use negative value: {}", value));
    const auto transparency = (1.0 - alpha()) * value;
    if (transparency >= 1.0)
        set_transparency(1.0);
    else
        set_transparency(transparency);
}

// ----------------------------------------------------------------------

//   // http://stackoverflow.com/questions/470690/how-to-automatically-generate-n-distinct-colors (search for kellysMaxContrastSet)
// const Color::value_type Color::DistinctColors[] = {
//     0xA6BDD7, //Very Light Blue
//     0xC10020, //Vivid Red
//     0xFFB300, //Vivid Yellow
//     0x803E75, //Strong Purple
//     0xFF6800, //Vivid Orange
//     0xCEA262, //Grayish Yellow
//       //0x817066, //Medium Gray

//       //The following will not be good for people with defective color vision
//     0x007D34, //Vivid Green
//     0xF6768E, //Strong Purplish Pink
//     0x00538A, //Strong Blue
//     0xFF7A5C, //Strong Yellowish Pink
//     0x53377A, //Strong Violet
//     0xFF8E00, //Vivid Orange Yellow
//     0xB32851, //Strong Purplish Red
//     0xF4C800, //Vivid Greenish Yellow
//     0x7F180D, //Strong Reddish Brown
//     0x93AA00, //Vivid Yellowish Green
//     0x593315, //Deep Yellowish Brown
//     0xF13A13, //Vivid Reddish Orange
//     0x232C16, //Dark Olive Green
// };

// std::vector<std::string> Color::distinct_colors()
// {
//     std::vector<std::string> result(std::end(DistinctColors) - std::begin(DistinctColors));
//     std::transform(std::begin(DistinctColors), std::end(DistinctColors), result.begin(), [](auto c) { return Color(c).to_string(); });
//     return result;
// }

// ----------------------------------------------------------------------

static const char* ana_colors[] =
{
    "#03569b",                  // dark blue
    "#e72f27",                  // dark red
    "#ffc808",                  // yellow
    "#a2b324",                  // dark green
    "#a5b8c7",                  // grey
    "#049457",                  // green
    "#f1b066",                  // pale orange
    "#742f32",                  // brown
    "#9e806e",                  // brown
    "#75ada9",                  // turquoise
    "#675b2c",
    "#a020f0",
    "#8b8989",
    "#e9a390",
    "#dde8cf",
    "#00939f",
};

static const char* google_maps_2017_colors[] =
{
    "#FF0000",
    "#4B96F3",
    "#4DB546",
    "#FFFF00",
    "#FF8000",
    "#15B5C7",
    "#F98182",
    "#7B9EB1",
    "#8F9CE2",
    "#0FBDFF",
};

std::vector<std::string> Color::distinct_s(distinct_t dtype)
{
    switch (dtype) {
      case distinct_t::Ana:
          return {std::begin(ana_colors), std::end(ana_colors)};
      case distinct_t::GoogleMaps:
          return {std::begin(google_maps_2017_colors), std::end(google_maps_2017_colors)};
    }
    return {std::begin(ana_colors), std::end(ana_colors)};
}

std::vector<Color> Color::distinct(distinct_t dtype)
{
    switch (dtype) {
      case distinct_t::Ana:
          return {std::begin(ana_colors), std::end(ana_colors)};
      case distinct_t::GoogleMaps:
          return {std::begin(google_maps_2017_colors), std::end(google_maps_2017_colors)};
    }
    return {std::begin(ana_colors), std::end(ana_colors)};
}

Color Color::distinct(size_t offset, distinct_t dtype)
{
    switch (dtype) {
      case distinct_t::Ana:
          return *(std::begin(ana_colors) + offset);
      case distinct_t::GoogleMaps:
          return *(std::begin(google_maps_2017_colors) + offset);
    }
    return *(std::begin(ana_colors) + offset);
}

// ----------------------------------------------------------------------

Color Color::perceptually_uniform_heatmap(size_t total_colors, size_t color_index)
{
    if (color_index >= total_colors)
        color_index = total_colors - 1;
    const auto step = static_cast<double>(_internal::sScaleColorPerceptualViridis.size()) / static_cast<double>(total_colors - 1);
    const auto offset = static_cast<size_t>(std::lround(color_index * step));
    if (offset >= _internal::sScaleColorPerceptualViridis.size())
        return _internal::sScaleColorPerceptualViridis.back();
    else
        return _internal::sScaleColorPerceptualViridis[offset];
}

// ----------------------------------------------------------------------

using namespace std::string_literals;
using namespace std::string_view_literals;

inline Color continent_color(std::string_view continent, const continent_colors_t& data)
{
    if (auto found = data.find(continent); found != data.end())
        return found->second;
    else if (auto found_unknown = data.find("UNKNOWN"sv); found_unknown != data.end())
        return found_unknown->second;
    else
        return PINK;
}


#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

static const continent_colors_t sContinentColors = {
    {"EUROPE"s,            0x00FF00},
    {"CENTRAL-AMERICA"s,   0xAAF9FF},
    {"MIDDLE-EAST"s,       0x8000FF},
    {"NORTH-AMERICA"s,     0x00008B},
    {"AFRICA"s,            0xFF8000},
    {"ASIA"s,              0xFF0000},
    {"RUSSIA"s,            0xB03060},
    {"AUSTRALIA-OCEANIA"s, 0xFF69B4},
    {"SOUTH-AMERICA"s,     0x40E0D0},
    {"ANTARCTICA"s,        0x808080},
    {"CHINA-SOUTH"s,       0xFF0000},
    {"CHINA-NORTH"s,       0x6495ED},
    {"CHINA-UNKNOWN"s,     0x808080},
    {"UNKNOWN"s,           0x808080},
};

#pragma GCC diagnostic pop

const continent_colors_t& continent_colors()
{
    return sContinentColors;

} // continent_colors

Color continent_color(std::string_view continent)
{
    return continent_color(continent, sContinentColors);
}

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

static const continent_colors_t sContinentColorsDark = {
    {"EUROPE"s,            0x00A800},
    {"CENTRAL-AMERICA"s,   0x70A4A8},
    {"MIDDLE-EAST"s,       0x8000FF},
    {"NORTH-AMERICA"s,     0x00008B},
    {"AFRICA"s,            0xFF8000},
    {"ASIA"s,              0xFF0000},
    {"RUSSIA"s,            0xB03060},
    {"AUSTRALIA-OCEANIA"s, 0xFF69B4},
    {"SOUTH-AMERICA"s,     0x40E0D0},
    {"ANTARCTICA"s,        0x808080},
    {"CHINA-SOUTH"s,       0xFF0000},
    {"CHINA-NORTH"s,       0x6495ED},
    {"CHINA-UNKNOWN"s,     0x808080},
    {"UNKNOWN"s,           0x808080},
};

#pragma GCC diagnostic pop

const continent_colors_t& continent_colors_dark()
{
    return sContinentColorsDark;

} // continent_colors_dark

Color continent_color_dark(std::string_view continent)
{
    return continent_color(continent, sContinentColorsDark);
}

// ----------------------------------------------------------------------

namespace _internal
{
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif
    const std::map<uint32_t, std::string> sColorToName = {
        {0xFF000000, "transparent"},
        {0x000000,   "black"},
        {0xFFFFFF,   "white"},
        {0xFF0000,   "red"},
        {0x00FF00,   "green"},
        {0x0000FF,   "blue"},
        {0x00FFFF,   "cyan"},
        {0xFF00FF,   "magenta"},
        {0xFFFF00,   "yellow"},
        {0xA52A2A,   "brown"},
        {0x6495ED,   "cornflowerblue"},
        {0xFFD700,   "gold"},
        {0xFFA500,   "orange"},
        {0xFFC0CB,   "pink"},
        {0xA020F0,   "purple"},
    };

    const std::map<std::string, uint32_t> sNameToColor = {
        {"transparent", 0xFF000000},
        {"black", 0x000000},
        {"white", 0xFFFFFF},

        {"ana1", 0x03569b},  // dark blue
        {"ana2", 0xe72f27},  // dark red
        {"ana3", 0xffc808},  // yellow
        {"ana4", 0xa2b324},  // dark green
        {"ana5", 0xa5b8c7},  // grey
        {"ana6", 0x049457},  // green
        {"ana7", 0xf1b066},  // pale orange
        {"ana8", 0x742f32},  // brown
        {"ana9", 0x9e806e},  // brown
        {"ana10", 0x75ada9}, // turquoise
        {"ana11", 0x675b2c},
        {"ana12", 0xa020f0},
        {"ana13", 0x8b8989},
        {"ana14", 0xe9a390},
        {"ana15", 0xdde8cf},
        {"ana16", 0x00939f},

        {"gmap1", 0xFF0000},
        {"gmap2", 0x4B96F3},
        {"gmap3", 0x4DB546},
        {"gmap4", 0xFFFF00},
        {"gmap5", 0xFF8000},
        {"gmap6", 0x15B5C7},
        {"gmap7", 0xF98182},
        {"gmap8", 0x7B9EB1},
        {"gmap9", 0x8F9CE2},
        {"gmap10", 0x0FBDFF},

        // /usr/X11/share/X11/rgb.txt
        {"alice blue", 0xF0F8FF},
        {"aliceblue", 0xF0F8FF},
        {"antique white", 0xFAEBD7},
        {"antiquewhite", 0xFAEBD7},
        {"antiquewhite1", 0xFFEFDB},
        {"antiquewhite2", 0xEEDFCC},
        {"antiquewhite3", 0xCDC0B0},
        {"antiquewhite4", 0x8B8378},
        {"aquamarine", 0x7FFFD4},
        {"aquamarine1", 0x7FFFD4},
        {"aquamarine2", 0x76EEC6},
        {"aquamarine3", 0x66CDAA},
        {"aquamarine4", 0x458B74},
        {"azure", 0xF0FFFF},
        {"azure1", 0xF0FFFF},
        {"azure2", 0xE0EEEE},
        {"azure3", 0xC1CDCD},
        {"azure4", 0x838B8B},
        {"beige", 0xF5F5DC},
        {"bisque", 0xFFE4C4},
        {"bisque1", 0xFFE4C4},
        {"bisque2", 0xEED5B7},
        {"bisque3", 0xCDB79E},
        {"bisque4", 0x8B7D6B},
        {"blanched almond", 0xFFEBCD},
        {"blanchedalmond", 0xFFEBCD},
        {"blue", 0x0000FF},
        {"blue violet", 0x8A2BE2},
        {"blue1", 0x0000FF},
        {"blue2", 0x0000EE},
        {"blue3", 0x0000CD},
        {"blue4", 0x00008B},
        {"blueviolet", 0x8A2BE2},
        {"brown", 0xA52A2A},
        {"brown1", 0xFF4040},
        {"brown2", 0xEE3B3B},
        {"brown3", 0xCD3333},
        {"brown4", 0x8B2323},
        {"burlywood", 0xDEB887},
        {"burlywood1", 0xFFD39B},
        {"burlywood2", 0xEEC591},
        {"burlywood3", 0xCDAA7D},
        {"burlywood4", 0x8B7355},
        {"cadet blue", 0x5F9EA0},
        {"cadetblue", 0x5F9EA0},
        {"cadetblue1", 0x98F5FF},
        {"cadetblue2", 0x8EE5EE},
        {"cadetblue3", 0x7AC5CD},
        {"cadetblue4", 0x53868B},
        {"chartreuse", 0x7FFF00},
        {"chartreuse1", 0x7FFF00},
        {"chartreuse2", 0x76EE00},
        {"chartreuse3", 0x66CD00},
        {"chartreuse4", 0x458B00},
        {"chocolate", 0xD2691E},
        {"chocolate1", 0xFF7F24},
        {"chocolate2", 0xEE7621},
        {"chocolate3", 0xCD661D},
        {"chocolate4", 0x8B4513},
        {"coral", 0xFF7F50},
        {"coral1", 0xFF7256},
        {"coral2", 0xEE6A50},
        {"coral3", 0xCD5B45},
        {"coral4", 0x8B3E2F},
        {"cornflower blue", 0x6495ED},
        {"cornflowerblue", 0x6495ED},
        {"cornflowerbluelight", 0xAAC5F5},
        {"cornsilk", 0xFFF8DC},
        {"cornsilk1", 0xFFF8DC},
        {"cornsilk2", 0xEEE8CD},
        {"cornsilk3", 0xCDC8B1},
        {"cornsilk4", 0x8B8878},
        {"cyan", 0x00FFFF},
        {"cyan1", 0x00FFFF},
        {"cyan2", 0x00EEEE},
        {"cyan3", 0x00CDCD},
        {"cyan4", 0x008B8B},
        {"dark blue", 0x00008B},
        {"dark cyan", 0x008B8B},
        {"dark goldenrod", 0xB8860B},
        {"dark gray", 0xA9A9A9},
        {"dark green", 0x006400},
        {"dark grey", 0xA9A9A9},
        {"dark khaki", 0xBDB76B},
        {"dark magenta", 0x8B008B},
        {"dark olive green", 0x556B2F},
        {"dark orange", 0xFF8C00},
        {"dark orchid", 0x9932CC},
        {"dark red", 0x8B0000},
        {"dark salmon", 0xE9967A},
        {"dark sea green", 0x8FBC8F},
        {"dark slate blue", 0x483D8B},
        {"dark slate gray", 0x2F4F4F},
        {"dark slate grey", 0x2F4F4F},
        {"dark turquoise", 0x00CED1},
        {"dark violet", 0x9400D3},
        {"darkblue", 0x00008B},
        {"darkcyan", 0x008B8B},
        {"darkgoldenrod", 0xB8860B},
        {"darkgoldenrod1", 0xFFB90F},
        {"darkgoldenrod2", 0xEEAD0E},
        {"darkgoldenrod3", 0xCD950C},
        {"darkgoldenrod4", 0x8B6508},
        {"darkgray", 0xA9A9A9},
        {"darkgreen", 0x006400},
        {"darkgrey", 0xA9A9A9},
        {"darkkhaki", 0xBDB76B},
        {"darkmagenta", 0x8B008B},
        {"darkolivegreen", 0x556B2F},
        {"darkolivegreen1", 0xCAFF70},
        {"darkolivegreen2", 0xBCEE68},
        {"darkolivegreen3", 0xA2CD5A},
        {"darkolivegreen4", 0x6E8B3D},
        {"darkorange", 0xFF8C00},
        {"darkorange1", 0xFF7F00},
        {"darkorange2", 0xEE7600},
        {"darkorange3", 0xCD6600},
        {"darkorange4", 0x8B4500},
        {"darkorchid", 0x9932CC},
        {"darkorchid1", 0xBF3EFF},
        {"darkorchid2", 0xB23AEE},
        {"darkorchid3", 0x9A32CD},
        {"darkorchid4", 0x68228B},
        {"darkred", 0x8B0000},
        {"darksalmon", 0xE9967A},
        {"darkseagreen", 0x8FBC8F},
        {"darkseagreen1", 0xC1FFC1},
        {"darkseagreen2", 0xB4EEB4},
        {"darkseagreen3", 0x9BCD9B},
        {"darkseagreen4", 0x698B69},
        {"darkslateblue", 0x483D8B},
        {"darkslategray", 0x2F4F4F},
        {"darkslategray1", 0x97FFFF},
        {"darkslategray2", 0x8DEEEE},
        {"darkslategray3", 0x79CDCD},
        {"darkslategray4", 0x528B8B},
        {"darkslategrey", 0x2F4F4F},
        {"darkturquoise", 0x00CED1},
        {"darkviolet", 0x9400D3},
        {"deep pink", 0xFF1493},
        {"deep sky blue", 0x00BFFF},
        {"deeppink", 0xFF1493},
        {"deeppink1", 0xFF1493},
        {"deeppink2", 0xEE1289},
        {"deeppink3", 0xCD1076},
        {"deeppink4", 0x8B0A50},
        {"deepskyblue", 0x00BFFF},
        {"deepskyblue1", 0x00BFFF},
        {"deepskyblue2", 0x00B2EE},
        {"deepskyblue3", 0x009ACD},
        {"deepskyblue4", 0x00688B},
        {"dim gray", 0x696969},
        {"dim grey", 0x696969},
        {"dimgray", 0x696969},
        {"dimgrey", 0x696969},
        {"dodger blue", 0x1E90FF},
        {"dodgerblue", 0x1E90FF},
        {"dodgerblue1", 0x1E90FF},
        {"dodgerblue2", 0x1C86EE},
        {"dodgerblue3", 0x1874CD},
        {"dodgerblue4", 0x104E8B},
        {"firebrick", 0xB22222},
        {"firebrick1", 0xFF3030},
        {"firebrick2", 0xEE2C2C},
        {"firebrick3", 0xCD2626},
        {"firebrick4", 0x8B1A1A},
        {"floral white", 0xFFFAF0},
        {"floralwhite", 0xFFFAF0},
        {"forest green", 0x228B22},
        {"forestgreen", 0x228B22},
        {"gainsboro", 0xDCDCDC},
        {"ghost white", 0xF8F8FF},
        {"ghostwhite", 0xF8F8FF},
        {"gold", 0xFFD700},
        {"gold1", 0xFFD700},
        {"gold2", 0xEEC900},
        {"gold3", 0xCDAD00},
        {"gold4", 0x8B7500},
        {"goldenrod", 0xDAA520},
        {"goldenrod1", 0xFFC125},
        {"goldenrod2", 0xEEB422},
        {"goldenrod3", 0xCD9B1D},
        {"goldenrod4", 0x8B6914},
        {"gray", 0xBEBEBE},
        {"gray0", 0x000000},
        {"gray1", 0x030303},
        {"gray10", 0x1A1A1A},
        {"gray100", 0xFFFFFF},
        {"gray11", 0x1C1C1C},
        {"gray12", 0x1F1F1F},
        {"gray13", 0x212121},
        {"gray14", 0x242424},
        {"gray15", 0x262626},
        {"gray16", 0x292929},
        {"gray17", 0x2B2B2B},
        {"gray18", 0x2E2E2E},
        {"gray19", 0x303030},
        {"gray2", 0x050505},
        {"gray20", 0x333333},
        {"gray21", 0x363636},
        {"gray22", 0x383838},
        {"gray23", 0x3B3B3B},
        {"gray24", 0x3D3D3D},
        {"gray25", 0x404040},
        {"gray26", 0x424242},
        {"gray27", 0x454545},
        {"gray28", 0x474747},
        {"gray29", 0x4A4A4A},
        {"gray3", 0x080808},
        {"gray30", 0x4D4D4D},
        {"gray31", 0x4F4F4F},
        {"gray32", 0x525252},
        {"gray33", 0x545454},
        {"gray34", 0x575757},
        {"gray35", 0x595959},
        {"gray36", 0x5C5C5C},
        {"gray37", 0x5E5E5E},
        {"gray38", 0x616161},
        {"gray39", 0x636363},
        {"gray4", 0x0A0A0A},
        {"gray40", 0x666666},
        {"gray41", 0x696969},
        {"gray42", 0x6B6B6B},
        {"gray43", 0x6E6E6E},
        {"gray44", 0x707070},
        {"gray45", 0x737373},
        {"gray46", 0x757575},
        {"gray47", 0x787878},
        {"gray48", 0x7A7A7A},
        {"gray49", 0x7D7D7D},
        {"gray5", 0x0D0D0D},
        {"gray50", 0x7F7F7F},
        {"gray51", 0x828282},
        {"gray52", 0x858585},
        {"gray53", 0x878787},
        {"gray54", 0x8A8A8A},
        {"gray55", 0x8C8C8C},
        {"gray56", 0x8F8F8F},
        {"gray57", 0x919191},
        {"gray58", 0x949494},
        {"gray59", 0x969696},
        {"gray6", 0x0F0F0F},
        {"gray60", 0x999999},
        {"gray61", 0x9C9C9C},
        {"gray62", 0x9E9E9E},
        {"gray63", 0xA1A1A1},
        {"gray64", 0xA3A3A3},
        {"gray65", 0xA6A6A6},
        {"gray66", 0xA8A8A8},
        {"gray67", 0xABABAB},
        {"gray68", 0xADADAD},
        {"gray69", 0xB0B0B0},
        {"gray7", 0x121212},
        {"gray70", 0xB3B3B3},
        {"gray71", 0xB5B5B5},
        {"gray72", 0xB8B8B8},
        {"gray73", 0xBABABA},
        {"gray74", 0xBDBDBD},
        {"gray75", 0xBFBFBF},
        {"gray76", 0xC2C2C2},
        {"gray77", 0xC4C4C4},
        {"gray78", 0xC7C7C7},
        {"gray79", 0xC9C9C9},
        {"gray8", 0x141414},
        {"gray80", 0xCCCCCC},
        {"gray81", 0xCFCFCF},
        {"gray82", 0xD1D1D1},
        {"gray83", 0xD4D4D4},
        {"gray84", 0xD6D6D6},
        {"gray85", 0xD9D9D9},
        {"gray86", 0xDBDBDB},
        {"gray87", 0xDEDEDE},
        {"gray88", 0xE0E0E0},
        {"gray89", 0xE3E3E3},
        {"gray9", 0x171717},
        {"gray90", 0xE5E5E5},
        {"gray91", 0xE8E8E8},
        {"gray92", 0xEBEBEB},
        {"gray93", 0xEDEDED},
        {"gray94", 0xF0F0F0},
        {"gray95", 0xF2F2F2},
        {"gray96", 0xF5F5F5},
        {"gray97", 0xF7F7F7},
        {"gray98", 0xFAFAFA},
        {"gray99", 0xFCFCFC},
        {"green", 0x00FF00},
        {"green yellow", 0xADFF2F},
        {"green1", 0x00FF00},
        {"green2", 0x00EE00},
        {"green3", 0x00CD00},
        {"green4", 0x008B00},
        {"greenyellow", 0xADFF2F},
        {"grey", 0xBEBEBE},
        {"grey0", 0x000000},
        {"grey1", 0x030303},
        {"grey10", 0x1A1A1A},
        //{"grey100", 0xFFFFFF},
        {"grey11", 0x1C1C1C},
        {"grey12", 0x1F1F1F},
        {"grey13", 0x212121},
        {"grey14", 0x242424},
        {"grey15", 0x262626},
        {"grey16", 0x292929},
        {"grey17", 0x2B2B2B},
        {"grey18", 0x2E2E2E},
        {"grey19", 0x303030},
        {"grey2", 0x050505},
        {"grey20", 0x333333},
        {"grey21", 0x363636},
        {"grey22", 0x383838},
        {"grey23", 0x3B3B3B},
        {"grey24", 0x3D3D3D},
        {"grey25", 0x404040},
        {"grey26", 0x424242},
        {"grey27", 0x454545},
        {"grey28", 0x474747},
        {"grey29", 0x4A4A4A},
        {"grey3", 0x080808},
        {"grey30", 0x4D4D4D},
        {"grey31", 0x4F4F4F},
        {"grey32", 0x525252},
        {"grey33", 0x545454},
        {"grey34", 0x575757},
        {"grey35", 0x595959},
        {"grey36", 0x5C5C5C},
        {"grey37", 0x5E5E5E},
        {"grey38", 0x616161},
        {"grey39", 0x636363},
        {"grey4", 0x0A0A0A},
        {"grey40", 0x666666},
        {"grey41", 0x696969},
        {"grey42", 0x6B6B6B},
        {"grey43", 0x6E6E6E},
        {"grey44", 0x707070},
        {"grey45", 0x737373},
        {"grey46", 0x757575},
        {"grey47", 0x787878},
        {"grey48", 0x7A7A7A},
        {"grey49", 0x7D7D7D},
        {"grey5", 0x0D0D0D},
        {"grey50", 0x7F7F7F},
        {"grey51", 0x828282},
        {"grey52", 0x858585},
        {"grey53", 0x878787},
        {"grey54", 0x8A8A8A},
        {"grey55", 0x8C8C8C},
        {"grey56", 0x8F8F8F},
        {"grey57", 0x919191},
        {"grey58", 0x949494},
        {"grey59", 0x969696},
        {"grey6", 0x0F0F0F},
        {"grey60", 0x999999},
        {"grey61", 0x9C9C9C},
        {"grey62", 0x9E9E9E},
        {"grey63", 0xA1A1A1},
        {"grey64", 0xA3A3A3},
        {"grey65", 0xA6A6A6},
        {"grey66", 0xA8A8A8},
        {"grey67", 0xABABAB},
        {"grey68", 0xADADAD},
        {"grey69", 0xB0B0B0},
        {"grey7", 0x121212},
        {"grey70", 0xB3B3B3},
        {"grey71", 0xB5B5B5},
        {"grey72", 0xB8B8B8},
        {"grey73", 0xBABABA},
        {"grey74", 0xBDBDBD},
        {"grey75", 0xBFBFBF},
        {"grey76", 0xC2C2C2},
        {"grey77", 0xC4C4C4},
        {"grey78", 0xC7C7C7},
        {"grey79", 0xC9C9C9},
        {"grey8", 0x141414},
        {"grey80", 0xCCCCCC},
        {"grey81", 0xCFCFCF},
        {"grey82", 0xD1D1D1},
        {"grey83", 0xD4D4D4},
        {"grey84", 0xD6D6D6},
        {"grey85", 0xD9D9D9},
        {"grey86", 0xDBDBDB},
        {"grey87", 0xDEDEDE},
        {"grey88", 0xE0E0E0},
        {"grey89", 0xE3E3E3},
        {"grey9", 0x171717},
        {"grey90", 0xE5E5E5},
        {"grey91", 0xE8E8E8},
        {"grey92", 0xEBEBEB},
        {"grey93", 0xEDEDED},
        {"grey94", 0xF0F0F0},
        {"grey95", 0xF2F2F2},
        {"grey96", 0xF5F5F5},
        {"grey97", 0xF7F7F7},
        {"grey98", 0xFAFAFA},
        {"grey99", 0xFCFCFC},
        {"grid_default_color", 0xC0000000},
        {"honeydew", 0xF0FFF0},
        {"honeydew1", 0xF0FFF0},
        {"honeydew2", 0xE0EEE0},
        {"honeydew3", 0xC1CDC1},
        {"honeydew4", 0x838B83},
        {"hot pink", 0xFF69B4},
        {"hotpink", 0xFF69B4},
        {"hotpink1", 0xFF6EB4},
        {"hotpink2", 0xEE6AA7},
        {"hotpink3", 0xCD6090},
        {"hotpink4", 0x8B3A62},
        {"indian red", 0xCD5C5C},
        {"indianred", 0xCD5C5C},
        {"indianred1", 0xFF6A6A},
        {"indianred2", 0xEE6363},
        {"indianred3", 0xCD5555},
        {"indianred4", 0x8B3A3A},
        {"ivory", 0xFFFFF0},
        {"ivory1", 0xFFFFF0},
        {"ivory2", 0xEEEEE0},
        {"ivory3", 0xCDCDC1},
        {"ivory4", 0x8B8B83},
        {"khaki", 0xF0E68C},
        {"khaki1", 0xFFF68F},
        {"khaki2", 0xEEE685},
        {"khaki3", 0xCDC673},
        {"khaki4", 0x8B864E},
        {"lavender", 0xE6E6FA},
        {"lavender blush", 0xFFF0F5},
        {"lavenderblush", 0xFFF0F5},
        {"lavenderblush1", 0xFFF0F5},
        {"lavenderblush2", 0xEEE0E5},
        {"lavenderblush3", 0xCDC1C5},
        {"lavenderblush4", 0x8B8386},
        {"lawn green", 0x7CFC00},
        {"lawngreen", 0x7CFC00},
        {"lemon chiffon", 0xFFFACD},
        {"lemonchiffon", 0xFFFACD},
        {"lemonchiffon1", 0xFFFACD},
        {"lemonchiffon2", 0xEEE9BF},
        {"lemonchiffon3", 0xCDC9A5},
        {"lemonchiffon4", 0x8B8970},
        {"light blue", 0xADD8E6},
        {"light coral", 0xF08080},
        {"light cyan", 0xE0FFFF},
        {"light goldenrod", 0xEEDD82},
        {"light goldenrod yellow", 0xFAFAD2},
        {"light gray", 0xD3D3D3},
        {"light green", 0x90EE90},
        {"light grey", 0xD3D3D3},
        {"light pink", 0xFFB6C1},
        {"light salmon", 0xFFA07A},
        {"light sea green", 0x20B2AA},
        {"light sky blue", 0x87CEFA},
        {"light slate blue", 0x8470FF},
        {"light slate gray", 0x778899},
        {"light slate grey", 0x778899},
        {"light steel blue", 0xB0C4DE},
        {"light yellow", 0xFFFFE0},
        {"lightblue", 0xADD8E6},
        {"lightblue1", 0xBFEFFF},
        {"lightblue2", 0xB2DFEE},
        {"lightblue3", 0x9AC0CD},
        {"lightblue4", 0x68838B},
        {"lightcoral", 0xF08080},
        {"lightcyan", 0xE0FFFF},
        {"lightcyan1", 0xE0FFFF},
        {"lightcyan2", 0xD1EEEE},
        {"lightcyan3", 0xB4CDCD},
        {"lightcyan4", 0x7A8B8B},
        {"lightgoldenrod", 0xEEDD82},
        {"lightgoldenrod1", 0xFFEC8B},
        {"lightgoldenrod2", 0xEEDC82},
        {"lightgoldenrod3", 0xCDBE70},
        {"lightgoldenrod4", 0x8B814C},
        {"lightgoldenrodyellow", 0xFAFAD2},
        {"lightgray", 0xD3D3D3},
        {"lightgreen", 0x90EE90},
        {"lightgrey", 0xD3D3D3},
        {"lightpink", 0xFFB6C1},
        {"lightpink1", 0xFFAEB9},
        {"lightpink2", 0xEEA2AD},
        {"lightpink3", 0xCD8C95},
        {"lightpink4", 0x8B5F65},
        {"lightsalmon", 0xFFA07A},
        {"lightsalmon1", 0xFFA07A},
        {"lightsalmon2", 0xEE9572},
        {"lightsalmon3", 0xCD8162},
        {"lightsalmon4", 0x8B5742},
        {"lightseagreen", 0x20B2AA},
        {"lightskyblue", 0x87CEFA},
        {"lightskyblue1", 0xB0E2FF},
        {"lightskyblue2", 0xA4D3EE},
        {"lightskyblue3", 0x8DB6CD},
        {"lightskyblue4", 0x607B8B},
        {"lightslateblue", 0x8470FF},
        {"lightslategray", 0x778899},
        {"lightslategrey", 0x778899},
        {"lightsteelblue", 0xB0C4DE},
        {"lightsteelblue1", 0xCAE1FF},
        {"lightsteelblue2", 0xBCD2EE},
        {"lightsteelblue3", 0xA2B5CD},
        {"lightsteelblue4", 0x6E7B8B},
        {"lightyellow", 0xFFFFE0},
        {"lightyellow1", 0xFFFFE0},
        {"lightyellow2", 0xEEEED1},
        {"lightyellow3", 0xCDCDB4},
        {"lightyellow4", 0x8B8B7A},
        {"lime green", 0x32CD32},
        {"limegreen", 0x32CD32},
        {"linen", 0xFAF0E6},
        {"magenta", 0xFF00FF},
        {"magenta1", 0xFF00FF},
        {"magenta2", 0xEE00EE},
        {"magenta3", 0xCD00CD},
        {"magenta4", 0x8B008B},
        {"maroon", 0xB03060},
        {"maroon1", 0xFF34B3},
        {"maroon2", 0xEE30A7},
        {"maroon3", 0xCD2990},
        {"maroon4", 0x8B1C62},
        {"medium aquamarine", 0x66CDAA},
        {"medium blue", 0x0000CD},
        {"medium orchid", 0xBA55D3},
        {"medium purple", 0x9370DB},
        {"medium sea green", 0x3CB371},
        {"medium slate blue", 0x7B68EE},
        {"medium spring green", 0x00FA9A},
        {"medium turquoise", 0x48D1CC},
        {"medium violet red", 0xC71585},
        {"mediumaquamarine", 0x66CDAA},
        {"mediumblue", 0x0000CD},
        {"mediumorchid", 0xBA55D3},
        {"mediumorchid1", 0xE066FF},
        {"mediumorchid2", 0xD15FEE},
        {"mediumorchid3", 0xB452CD},
        {"mediumorchid4", 0x7A378B},
        {"mediumpurple", 0x9370DB},
        {"mediumpurple1", 0xAB82FF},
        {"mediumpurple2", 0x9F79EE},
        {"mediumpurple3", 0x8968CD},
        {"mediumpurple4", 0x5D478B},
        {"mediumseagreen", 0x3CB371},
        {"mediumslateblue", 0x7B68EE},
        {"mediumspringgreen", 0x00FA9A},
        {"mediumturquoise", 0x48D1CC},
        {"mediumvioletred", 0xC71585},
        {"midnight blue", 0x191970},
        {"midnightblue", 0x191970},
        {"mint cream", 0xF5FFFA},
        {"mintcream", 0xF5FFFA},
        {"misty rose", 0xFFE4E1},
        {"mistyrose", 0xFFE4E1},
        {"mistyrose1", 0xFFE4E1},
        {"mistyrose2", 0xEED5D2},
        {"mistyrose3", 0xCDB7B5},
        {"mistyrose4", 0x8B7D7B},
        {"moccasin", 0xFFE4B5},
        {"navajo white", 0xFFDEAD},
        {"navajowhite", 0xFFDEAD},
        {"navajowhite1", 0xFFDEAD},
        {"navajowhite2", 0xEECFA1},
        {"navajowhite3", 0xCDB38B},
        {"navajowhite4", 0x8B795E},
        {"navy", 0x000080},
        {"navy blue", 0x000080},
        {"navyblue", 0x000080},
        {"old lace", 0xFDF5E6},
        {"oldlace", 0xFDF5E6},
        {"olive drab", 0x6B8E23},
        {"olivedrab", 0x6B8E23},
        {"olivedrab1", 0xC0FF3E},
        {"olivedrab2", 0xB3EE3A},
        {"olivedrab3", 0x9ACD32},
        {"olivedrab4", 0x698B22},
        {"orange", 0xFFA500},
        {"orange red", 0xFF4500},
        {"orange1", 0xFFA500},
        {"orange2", 0xEE9A00},
        {"orange3", 0xCD8500},
        {"orange4", 0x8B5A00},
        {"orangered", 0xFF4500},
        {"orangered1", 0xFF4500},
        {"orangered2", 0xEE4000},
        {"orangered3", 0xCD3700},
        {"orangered4", 0x8B2500},
        {"orchid", 0xDA70D6},
        {"orchid1", 0xFF83FA},
        {"orchid2", 0xEE7AE9},
        {"orchid3", 0xCD69C9},
        {"orchid4", 0x8B4789},
        {"pale goldenrod", 0xEEE8AA},
        {"pale green", 0x98FB98},
        {"pale turquoise", 0xAFEEEE},
        {"pale violet red", 0xDB7093},
        {"palegoldenrod", 0xEEE8AA},
        {"palegreen", 0x98FB98},
        {"palegreen1", 0x9AFF9A},
        {"palegreen2", 0x90EE90},
        {"palegreen3", 0x7CCD7C},
        {"palegreen4", 0x548B54},
        {"paleturquoise", 0xAFEEEE},
        {"paleturquoise1", 0xBBFFFF},
        {"paleturquoise2", 0xAEEEEE},
        {"paleturquoise3", 0x96CDCD},
        {"paleturquoise4", 0x668B8B},
        {"palevioletred", 0xDB7093},
        {"palevioletred1", 0xFF82AB},
        {"palevioletred2", 0xEE799F},
        {"palevioletred3", 0xCD6889},
        {"palevioletred4", 0x8B475D},
        {"papaya whip", 0xFFEFD5},
        {"papayawhip", 0xFFEFD5},
        {"peach puff", 0xFFDAB9},
        {"peachpuff", 0xFFDAB9},
        {"peachpuff1", 0xFFDAB9},
        {"peachpuff2", 0xEECBAD},
        {"peachpuff3", 0xCDAF95},
        {"peachpuff4", 0x8B7765},
        {"peru", 0xCD853F},
        {"pink", 0xFFC0CB},
        {"pink1", 0xFFB5C5},
        {"pink2", 0xEEA9B8},
        {"pink3", 0xCD919E},
        {"pink4", 0x8B636C},
        {"plum", 0xDDA0DD},
        {"plum1", 0xFFBBFF},
        {"plum2", 0xEEAEEE},
        {"plum3", 0xCD96CD},
        {"plum4", 0x8B668B},
        {"powder blue", 0xB0E0E6},
        {"powderblue", 0xB0E0E6},
        {"purple", 0xA020F0},
        {"purple1", 0x9B30FF},
        {"purple2", 0x912CEE},
        {"purple3", 0x7D26CD},
        {"purple4", 0x551A8B},
        {"red", 0xFF0000},
        {"red1", 0xFF0000},
        {"red2", 0xEE0000},
        {"red3", 0xCD0000},
        {"red4", 0x8B0000},
        {"rosy brown", 0xBC8F8F},
        {"rosybrown", 0xBC8F8F},
        {"rosybrown1", 0xFFC1C1},
        {"rosybrown2", 0xEEB4B4},
        {"rosybrown3", 0xCD9B9B},
        {"rosybrown4", 0x8B6969},
        {"royal blue", 0x4169E1},
        {"royalblue", 0x4169E1},
        {"royalblue1", 0x4876FF},
        {"royalblue2", 0x436EEE},
        {"royalblue3", 0x3A5FCD},
        {"royalblue4", 0x27408B},
        {"saddle brown", 0x8B4513},
        {"saddlebrown", 0x8B4513},
        {"salmon", 0xFA8072},
        {"salmon1", 0xFF8C69},
        {"salmon2", 0xEE8262},
        {"salmon3", 0xCD7054},
        {"salmon4", 0x8B4C39},
        {"sandy brown", 0xF4A460},
        {"sandybrown", 0xF4A460},
        {"sea green", 0x2E8B57},
        {"seagreen", 0x2E8B57},
        {"seagreen1", 0x54FF9F},
        {"seagreen2", 0x4EEE94},
        {"seagreen3", 0x43CD80},
        {"seagreen4", 0x2E8B57},
        {"seashell", 0xFFF5EE},
        {"seashell1", 0xFFF5EE},
        {"seashell2", 0xEEE5DE},
        {"seashell3", 0xCDC5BF},
        {"seashell4", 0x8B8682},
        {"sienna", 0xA0522D},
        {"sienna1", 0xFF8247},
        {"sienna2", 0xEE7942},
        {"sienna3", 0xCD6839},
        {"sienna4", 0x8B4726},
        {"sky blue", 0x87CEEB},
        {"skyblue", 0x87CEEB},
        {"skyblue1", 0x87CEFF},
        {"skyblue2", 0x7EC0EE},
        {"skyblue3", 0x6CA6CD},
        {"skyblue4", 0x4A708B},
        {"slate blue", 0x6A5ACD},
        {"slate gray", 0x708090},
        {"slate grey", 0x708090},
        {"slateblue", 0x6A5ACD},
        {"slateblue1", 0x836FFF},
        {"slateblue2", 0x7A67EE},
        {"slateblue3", 0x6959CD},
        {"slateblue4", 0x473C8B},
        {"slategray", 0x708090},
        {"slategray1", 0xC6E2FF},
        {"slategray2", 0xB9D3EE},
        {"slategray3", 0x9FB6CD},
        {"slategray4", 0x6C7B8B},
        {"slategrey", 0x708090},
        {"snow", 0xFFFAFA},
        {"snow1", 0xFFFAFA},
        {"snow2", 0xEEE9E9},
        {"snow3", 0xCDC9C9},
        {"snow4", 0x8B8989},
        {"spring green", 0x00FF7F},
        {"springgreen", 0x00FF7F},
        {"springgreen1", 0x00FF7F},
        {"springgreen2", 0x00EE76},
        {"springgreen3", 0x00CD66},
        {"springgreen4", 0x008B45},
        {"steel blue", 0x4682B4},
        {"steelblue", 0x4682B4},
        {"steelblue1", 0x63B8FF},
        {"steelblue2", 0x5CACEE},
        {"steelblue3", 0x4F94CD},
        {"steelblue4", 0x36648B},
        {"tan", 0xD2B48C},
        {"tan1", 0xFFA54F},
        {"tan2", 0xEE9A49},
        {"tan3", 0xCD853F},
        {"tan4", 0x8B5A2B},
        {"thistle", 0xD8BFD8},
        {"thistle1", 0xFFE1FF},
        {"thistle2", 0xEED2EE},
        {"thistle3", 0xCDB5CD},
        {"thistle4", 0x8B7B8B},
        {"tomato", 0xFF6347},
        {"tomato1", 0xFF6347},
        {"tomato2", 0xEE5C42},
        {"tomato3", 0xCD4F39},
        {"tomato4", 0x8B3626},
        {"turquoise", 0x40E0D0},
        {"turquoise1", 0x00F5FF},
        {"turquoise2", 0x00E5EE},
        {"turquoise3", 0x00C5CD},
        {"turquoise4", 0x00868B},
        {"violet", 0xEE82EE},
        {"violet red", 0xD02090},
        {"violetred", 0xD02090},
        {"violetred1", 0xFF3E96},
        {"violetred2", 0xEE3A8C},
        {"violetred3", 0xCD3278},
        {"violetred4", 0x8B2252},
        {"wheat", 0xF5DEB3},
        {"wheat1", 0xFFE7BA},
        {"wheat2", 0xEED8AE},
        {"wheat3", 0xCDBA96},
        {"wheat4", 0x8B7E66},
        {"white smoke", 0xF5F5F5},
        {"whitesmoke", 0xF5F5F5},
        {"yellow", 0xFFFF00},
        {"yellow green", 0x9ACD32},
        {"yellow1", 0xFFFF00},
        {"yellow2", 0xEEEE00},
        {"yellow3", 0xCDCD00},
        {"yellow4", 0x8B8B00},
        {"yellowgreen", 0x9ACD32},

    };

    // ----------------------------------------------------------------------

    // Copyright (c) 2015, Politiken Journalism <emil.bay@pol.dk>

    // Permission to use, copy, modify, and/or distribute this software for any
    // purpose with or without fee is hereby granted, provided that the above
    // copyright notice and this permission notice appear in all copies.

    // THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    // WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    // MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    // ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    // WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    // ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    // OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

    // https://github.com/politiken-journalism/scale-color-perceptual/blob/master/hex/viridis.json

    std::array<uint32_t, 256> sScaleColorPerceptualViridis{
        0x440154, 0x440256, 0x450457, 0x450559, 0x46075a, 0x46085c, 0x460a5d, 0x460b5e, 0x470d60, 0x470e61, 0x471063, 0x471164, 0x471365, 0x481467, 0x481668, 0x481769,
        0x48186a, 0x481a6c, 0x481b6d, 0x481c6e, 0x481d6f, 0x481f70, 0x482071, 0x482173, 0x482374, 0x482475, 0x482576, 0x482677, 0x482878, 0x482979, 0x472a7a, 0x472c7a,
        0x472d7b, 0x472e7c, 0x472f7d, 0x46307e, 0x46327e, 0x46337f, 0x463480, 0x453581, 0x453781, 0x453882, 0x443983, 0x443a83, 0x443b84, 0x433d84, 0x433e85, 0x423f85,
        0x424086, 0x424186, 0x414287, 0x414487, 0x404588, 0x404688, 0x3f4788, 0x3f4889, 0x3e4989, 0x3e4a89, 0x3e4c8a, 0x3d4d8a, 0x3d4e8a, 0x3c4f8a, 0x3c508b, 0x3b518b,
        0x3b528b, 0x3a538b, 0x3a548c, 0x39558c, 0x39568c, 0x38588c, 0x38598c, 0x375a8c, 0x375b8d, 0x365c8d, 0x365d8d, 0x355e8d, 0x355f8d, 0x34608d, 0x34618d, 0x33628d,
        0x33638d, 0x32648e, 0x32658e, 0x31668e, 0x31678e, 0x31688e, 0x30698e, 0x306a8e, 0x2f6b8e, 0x2f6c8e, 0x2e6d8e, 0x2e6e8e, 0x2e6f8e, 0x2d708e, 0x2d718e, 0x2c718e,
        0x2c728e, 0x2c738e, 0x2b748e, 0x2b758e, 0x2a768e, 0x2a778e, 0x2a788e, 0x29798e, 0x297a8e, 0x297b8e, 0x287c8e, 0x287d8e, 0x277e8e, 0x277f8e, 0x27808e, 0x26818e,
        0x26828e, 0x26828e, 0x25838e, 0x25848e, 0x25858e, 0x24868e, 0x24878e, 0x23888e, 0x23898e, 0x238a8d, 0x228b8d, 0x228c8d, 0x228d8d, 0x218e8d, 0x218f8d, 0x21908d,
        0x21918c, 0x20928c, 0x20928c, 0x20938c, 0x1f948c, 0x1f958b, 0x1f968b, 0x1f978b, 0x1f988b, 0x1f998a, 0x1f9a8a, 0x1e9b8a, 0x1e9c89, 0x1e9d89, 0x1f9e89, 0x1f9f88,
        0x1fa088, 0x1fa188, 0x1fa187, 0x1fa287, 0x20a386, 0x20a486, 0x21a585, 0x21a685, 0x22a785, 0x22a884, 0x23a983, 0x24aa83, 0x25ab82, 0x25ac82, 0x26ad81, 0x27ad81,
        0x28ae80, 0x29af7f, 0x2ab07f, 0x2cb17e, 0x2db27d, 0x2eb37c, 0x2fb47c, 0x31b57b, 0x32b67a, 0x34b679, 0x35b779, 0x37b878, 0x38b977, 0x3aba76, 0x3bbb75, 0x3dbc74,
        0x3fbc73, 0x40bd72, 0x42be71, 0x44bf70, 0x46c06f, 0x48c16e, 0x4ac16d, 0x4cc26c, 0x4ec36b, 0x50c46a, 0x52c569, 0x54c568, 0x56c667, 0x58c765, 0x5ac864, 0x5cc863,
        0x5ec962, 0x60ca60, 0x63cb5f, 0x65cb5e, 0x67cc5c, 0x69cd5b, 0x6ccd5a, 0x6ece58, 0x70cf57, 0x73d056, 0x75d054, 0x77d153, 0x7ad151, 0x7cd250, 0x7fd34e, 0x81d34d,
        0x84d44b, 0x86d549, 0x89d548, 0x8bd646, 0x8ed645, 0x90d743, 0x93d741, 0x95d840, 0x98d83e, 0x9bd93c, 0x9dd93b, 0xa0da39, 0xa2da37, 0xa5db36, 0xa8db34, 0xaadc32,
        0xaddc30, 0xb0dd2f, 0xb2dd2d, 0xb5de2b, 0xb8de29, 0xbade28, 0xbddf26, 0xc0df25, 0xc2df23, 0xc5e021, 0xc8e020, 0xcae11f, 0xcde11d, 0xd0e11c, 0xd2e21b, 0xd5e21a,
        0xd8e219, 0xdae319, 0xdde318, 0xdfe318, 0xe2e418, 0xe5e419, 0xe7e419, 0xeae51a, 0xece51b, 0xefe51c, 0xf1e51d, 0xf4e61e, 0xf6e620, 0xf8e621, 0xfbe723, 0xfde725
    };

#pragma GCC diagnostic pop
} // namespace _internal

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
