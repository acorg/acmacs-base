#pragma once

#include "acmacs-base/color.hh"

// ----------------------------------------------------------------------

namespace acmacs::color
{
    Color perceptually_uniform_heatmap(size_t total_colors, size_t color_index);
    std::vector<Color> bezier_gradient(Color c1, Color c2, Color c3, size_t output_size);

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
