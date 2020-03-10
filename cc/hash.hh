#pragma once

#pragma GCC diagnostic push

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wextra-semi"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wunused-template"
#endif

#include "acmacs-base/xxhash.hpp"

#pragma GCC diagnostic pop

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    inline std::string hash(std::string_view source)
    {
        return fmt::format("{:08X}", xxh::xxhash<32>(source));
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
