#pragma once

#pragma GCC diagnostic push

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#pragma GCC diagnostic ignored "-Wused-but-marked-unused"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wused-but-marked-unused"
#pragma GCC diagnostic ignored "-Wextra-semi-stmt"
#pragma GCC diagnostic ignored "-Wold-style-cast"
// M1
#pragma GCC diagnostic ignored "-Wreserved-identifier"
// #pragma GCC diagnostic ignored ""
#endif

// #include "acmacs-base/xxhash.hpp"

#define XXH_INLINE_ALL
#include "acmacs-base/xxhash.h"

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    inline std::string hash(std::string_view source)
    {
        return fmt::format("{:08X}", XXH32(source.data(), source.size(), 0));
    }
}

// ----------------------------------------------------------------------

#pragma GCC diagnostic pop
