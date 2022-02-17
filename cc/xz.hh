#pragma once

#include <cstring>
#include <string>
#include <string_view>

// ----------------------------------------------------------------------

namespace acmacs::file
{
    namespace xz_internal
    {
        const unsigned char sXzSig[] = { 0xFD, '7', 'z', 'X', 'Z', 0x00 };
    }

      // ----------------------------------------------------------------------

    inline bool xz_compressed(const char* input)
    {
        return std::memcmp(input, xz_internal::sXzSig, sizeof(xz_internal::sXzSig)) == 0;
    }

      // ----------------------------------------------------------------------

    std::string xz_compress(std::string_view input);
    std::string xz_decompress(std::string_view input);

} // namespace acmacs::file

// ----------------------------------------------------------------------
