#include "acmacs-base/color-continent.hh"

// ----------------------------------------------------------------------

inline Color continent_color(std::string_view continent, const acmacs::continent_colors_t& data)
{
    using namespace std::string_view_literals;
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

using namespace std::string_literals;

static const acmacs::continent_colors_t sContinentColors = {
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

static const acmacs::continent_colors_t sContinentColorsDark = {
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

// ----------------------------------------------------------------------

const acmacs::continent_colors_t& acmacs::continent_colors()
{
    return sContinentColors;

} // acmacs::continent_colors

// ----------------------------------------------------------------------

const acmacs::continent_colors_t& acmacs::continent_colors_dark()
{
    return sContinentColorsDark;

} // acmacs::continent_colors_dark

// ----------------------------------------------------------------------

Color acmacs::continent_color(std::string_view continent)
{
    return continent_color(continent, sContinentColors);

} // acmacs::continent_color

// ----------------------------------------------------------------------

Color acmacs::continent_color_dark(std::string_view continent)
{
    return continent_color(continent, sContinentColorsDark);

} // acmacs::continent_color_dark

// ----------------------------------------------------------------------
