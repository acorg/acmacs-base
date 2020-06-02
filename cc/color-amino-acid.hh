#pragma once

#include "acmacs-base/color.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    Color amino_acid_color(char aa);
    Color nucleotide_color(char nuc);

    std::string amino_acid_nucleotide_color_css();

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
