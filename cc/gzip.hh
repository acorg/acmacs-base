#pragma once

#include <string>
#include <string_view>
#include <zlib.h>

// ----------------------------------------------------------------------

namespace acmacs::file
{
    namespace gzip_internal
    {
        constexpr const unsigned char sGzipSig[] = { 0x1F, 0x8B };

    } // namespace gzip_internal

      // ----------------------------------------------------------------------

    inline bool gzip_compressed(const char* input)
    {
        return std::memcmp(input, gzip_internal::sGzipSig, sizeof(gzip_internal::sGzipSig)) == 0;
    }

      // ----------------------------------------------------------------------

    inline std::string gzip_compress(std::string /*input*/)
    {
        throw std::runtime_error("acmacs::file::gzip_compress not implemented");
        // lzma_stream strm = LZMA_STREAM_INIT; /* alloc and init lzma_stream struct */
        // if (lzma_easy_encoder(&strm, 9 | LZMA_PRESET_EXTREME, LZMA_CHECK_CRC64) != LZMA_OK) {
        //     throw std::runtime_error("lzma compression failed 1");
        // }
        // return xz_internal::process(&strm, input);
    }

      // ----------------------------------------------------------------------

    inline std::string gzip_decompress(std::string_view input)
    {
        constexpr ssize_t BufSize = 16384;
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.next_in = reinterpret_cast<decltype(strm.next_in)>(const_cast<char*>(input.data()));
        strm.total_in = strm.avail_in = static_cast<decltype(strm.avail_in)>(input.size());

        if (inflateInit2(&strm, 15 + 32) != Z_OK) // 15 window bits, and the +32 tells zlib to to detect if using gzip or zlib
            throw std::runtime_error("gzip decompression failed during initialization");

        try {
            std::string output(BufSize, ' ');
            ssize_t offset = 0;
            for (;;) {
                strm.next_out = reinterpret_cast<decltype(strm.next_out)>(output.data() + offset);
                strm.avail_out = BufSize;
                auto const r = inflate(&strm, Z_NO_FLUSH);
                if (r == Z_OK) {
                    if (strm.avail_out > 0)
                        throw std::runtime_error("gzip decompression failed: unexpected end of input");
                    offset += BufSize;
                    output.resize(static_cast<size_t>(offset + BufSize));
                }
                else if (r == Z_STREAM_END) {
                    output.resize(static_cast<size_t>(offset + BufSize) - strm.avail_out);
                    break;
                }
                else {
                    throw std::runtime_error("gzip decompression failed, code: " + std::to_string(r));
                }
            }
            inflateEnd(&strm);
            return output;
        }
        catch (std::exception&) {
            inflateEnd(&strm);
            throw;
        }
    }

} // namespace acmacs::file

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
