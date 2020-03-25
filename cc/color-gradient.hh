#pragma once

#include "acmacs-base/color.hh"

// ----------------------------------------------------------------------

namespace acmacs::color
{
    Color perceptually_uniform_heatmap(size_t total_colors, size_t color_index);

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
