#pragma once

#include "acmacs-base/flat-map.hh"
#include "acmacs-base/color.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    using continent_colors_t = acmacs::small_map_with_unique_keys_t<std::string, Color>;

    const continent_colors_t& continent_colors();
    const continent_colors_t& continent_colors_dark();
    Color continent_color(std::string_view continent);
    Color continent_color_dark(std::string_view continent);

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
