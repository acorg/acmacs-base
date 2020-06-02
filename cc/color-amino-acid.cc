#include "acmacs-base/color-amino-acid.hh"

// ----------------------------------------------------------------------

using ac = std::pair<char, Color>;
static constexpr const std::array amino_acid_colors{
    ac{'D', 0xF7CF08}, // negative
    ac{'E', 0xCD8D00},

    ac{'R', 0xB2A1FF}, // positive
    ac{'H', 0x7571BC},
    ac{'K', 0x440FAE},

    ac{'N', 0x18E0F7}, // polar uncharged
    ac{'Q', 0x0080FF},
    ac{'S', 0x0F52BA},
    ac{'T', 0x000080},

    ac{'C', 0x00F000}, // special
    ac{'G', 0x4CBB1 },
    ac{'P', 0x1A4F30},

    ac{'A', 0xEB1919}, // hydrophobic
    ac{'I', 0xF93BBB},
    ac{'L', 0xFF2976},
    ac{'M', 0xFF5A3D},
    ac{'F', 0xC34822},
    ac{'W', 0xA30000},
    ac{'Y', 0x941651},
    ac{'V', 0x681226},

    ac{'X', 0x808080},
};

// // first variant with C being too bright
// const std::array amino_acid_colors{
//      ac{'A', 0xA30000}, // hydrophobic
//      ac{'C', 0xFFFF29}, // special
//      ac{'D', 0x4CBB17}, // negative
//      ac{'E', 0x1A4F30}, // negative
//      ac{'F', 0x681226}, // hydrophobic
//      ac{'G', 0xF7CF08}, // special
//      ac{'H', 0x7571BC}, // positive
//      ac{'I', 0xFF5A3D}, // hydrophobic
//      ac{'K', 0x440FAE}, // positive
//      ac{'L', 0xC34822}, // hydrophobic
//      ac{'M', 0xEB1919}, // hydrophobic
//      ac{'N', 0x18E0F7}, // polar uncharged
//      ac{'P', 0xCD8D00}, // special
//      ac{'Q', 0x0080FF}, // polar uncharged
//      ac{'R', 0xB2A1FF}, // positive
//      ac{'S', 0x0F52BA}, // polar uncharged
//      ac{'T', 0x000080}, // polar uncharged
//      ac{'V', 0xF9BDED}, // hydrophobic
//      ac{'W', 0xC32290}, // hydrophobic
//      ac{'Y', 0xFF2976}, // hydrophobic
//      ac{'X', 0x808080},
// };

// ----------------------------------------------------------------------

static constexpr const std::array nucleotide_colors{
     ac{'A', 0x0080FF},
     ac{'C', 0x4CBB17},
     ac{'G', 0xEB1919},
     ac{'T', 0xCD8D00},
};

// ----------------------------------------------------------------------

std::string acmacs::amino_acid_nucleotide_color_css()
{
    fmt::memory_buffer out;
    for (const auto& en : amino_acid_colors)
        fmt::format_to(out, ".aa{} {{color: {:#}}}\n", en.first, en.second);
    for (const auto& en : nucleotide_colors)
        fmt::format_to(out, ".nuc{} {{color: {:#}}}\n", en.first, en.second);
    return fmt::to_string(out);

} // acmacs::amino_acid_nucleotide_color_css

// ----------------------------------------------------------------------

Color acmacs::amino_acid_color(char aa)
{
    if (const auto found = std::find_if(std::begin(amino_acid_colors), std::end(amino_acid_colors), [aa](const auto& en) { return aa == en.first; }); found != std::end(amino_acid_colors))
        return found->second;
    else
        return BLACK;

} // acmacs::amino_acid_color

// ----------------------------------------------------------------------

Color acmacs::nucleotide_color(char nuc)
{
    if (const auto found = std::find_if(std::begin(nucleotide_colors), std::end(nucleotide_colors), [nuc](const auto& en) { return nuc == en.first; }); found != std::end(amino_acid_colors))
        return found->second;
    else
        return BLACK;

} // acmacs::nucleotide_color

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
