#include <iostream>
#include <array>
#include <algorithm>

#include "acmacs-base/string.hh"
#include "acmacs-base/normalize.hh"

// ----------------------------------------------------------------------

static const std::array sVirusTypes = {
    std::pair{"H1", "A(H1N1)"},
    std::pair{"H3", "A(H3N2)"},
};

std::string acmacs::normalize_virus_type(std::string_view vt)
{
    const auto look_for = string::upper(vt);
    if (const auto found = std::find_if(std::begin(sVirusTypes), std::end(sVirusTypes), [look_for](const auto& entry) { return entry.first == look_for; }); found != std::end(sVirusTypes))
        return found->second;
    return look_for;

} // acmacs::normalize_virus_type

// ----------------------------------------------------------------------

static const std::array sLineages = {
    std::pair{"V", "VICTORIA"},
    std::pair{"VIC", "VICTORIA"},
    std::pair{"Y", "YAMAGATA"},
    std::pair{"YAM", "YAMAGATA"},
};

std::string acmacs::normalize_lineage(std::string_view lineage)
{
    const auto look_for = string::upper(lineage);
    if (const auto found = std::find_if(std::begin(sLineages), std::end(sLineages), [look_for](const auto& entry) { return entry.first == look_for; }); found != std::end(sLineages))
        return found->second;
    return look_for;

} // acmacs::normalize_lineage

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
