#include "acmacs-base/color-distinct.hh"

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
    "#FF0000",                  // gmap1
    "#4B96F3",                  // gmap2
    "#4DB546",                  // gmap3
    "#FFFF00",                  // gmap4
    "#FF8000",                  // gmap5
    "#15B5C7",                  // gmap6
    "#F98182",                  // gmap7
    "#7B9EB1",                  // gmap8
    "#8F9CE2",                  // gmap9
    "#0FBDFF",                  // gmap10
};

// ----------------------------------------------------------------------

std::vector<std::string> acmacs::color::distinct_s(distinct_t dtype)
{
    switch (dtype) {
      case distinct_t::Ana:
          return {std::begin(ana_colors), std::end(ana_colors)};
      case distinct_t::GoogleMaps:
          return {std::begin(google_maps_2017_colors), std::end(google_maps_2017_colors)};
    }
    return {std::begin(ana_colors), std::end(ana_colors)};

} // acmacs::color::distinct_s

// ----------------------------------------------------------------------

std::vector<Color> acmacs::color::distinct(distinct_t dtype)
{
    switch (dtype) {
      case distinct_t::Ana:
          return {std::begin(ana_colors), std::end(ana_colors)};
      case distinct_t::GoogleMaps:
          return {std::begin(google_maps_2017_colors), std::end(google_maps_2017_colors)};
    }
    return {std::begin(ana_colors), std::end(ana_colors)};

} // acmacs::color::distinct

// ----------------------------------------------------------------------

Color acmacs::color::distinct(size_t offset, distinct_t dtype)
{
    switch (dtype) {
      case distinct_t::Ana:
          return Color{*(std::begin(ana_colors) + offset)};
      case distinct_t::GoogleMaps:
          return Color{*(std::begin(google_maps_2017_colors) + offset)};
    }
    return Color{*(std::begin(ana_colors) + offset)};

} // acmacs::color::distinct

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
