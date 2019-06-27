#pragma once

#include "acmacs-base/named-type.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    using number_of_dimensions_t = named_size_t<struct number_of_dimensions_tag>;

    constexpr bool valid(number_of_dimensions_t nd) { return nd.get() > 0; }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
