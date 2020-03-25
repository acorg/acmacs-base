#pragma once

#include "acmacs-base/color.hh"

// ----------------------------------------------------------------------

namespace acmacs::color
{
    enum class distinct_t { Ana, GoogleMaps };

    std::vector<std::string> distinct_s(distinct_t dtype = distinct_t::Ana);
    std::vector<Color> distinct(distinct_t dtype = distinct_t::Ana);
    Color distinct(size_t offset, distinct_t dtype = distinct_t::Ana);

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
