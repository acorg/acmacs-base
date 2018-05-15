#pragma once

#include <string>
#include <string_view>
#include <cstring>

// ----------------------------------------------------------------------

namespace acmacs::file
{
    namespace gzip_internal
    {
        constexpr const unsigned char sGzipSig[] = { 0x1F, 0x8B };

    } // namespace gzip_internal

      // ----------------------------------------------------------------------

    inline bool gzip_compressed(const char* input) { return std::memcmp(input, gzip_internal::sGzipSig, sizeof(gzip_internal::sGzipSig)) == 0; }
    std::string gzip_compress(std::string_view input);
    std::string gzip_decompress(std::string_view input);

} // namespace acmacs::file

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
