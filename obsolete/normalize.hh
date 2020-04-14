#pragma once

#include <string>
#include <string_view>

namespace acmacs
{
    // h3 -> A(H3N2)
    std::string normalize_virus_type(std::string_view vt);

    // V -> VICTORIA
    std::string normalize_lineage(std::string_view lineage);

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
