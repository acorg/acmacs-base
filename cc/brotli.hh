#pragma once

#include <cstring>
#include <string>
#include <string_view>
#include <stdexcept>

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdocumentation-pedantic"
#pragma GCC diagnostic ignored "-Wdocumentation-unknown-command"
#endif

#include <brotli/decode.h>
#include <brotli/encode.h>

#pragma GCC diagnostic pop

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace acmacs::file
{
    struct BrotliError : public std::runtime_error { using std::runtime_error::runtime_error; };

      // ----------------------------------------------------------------------

    inline std::string brotli_compress(std::string_view input)
    {
        std::string output(std::min(input.size() / 2 + 1, 1024ul * 1024), 0);
        BrotliEncoderState* state = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
        size_t available_in = input.size();
        const auto* next_in = reinterpret_cast<const uint8_t*>(input.data());
        auto* next_out = reinterpret_cast<uint8_t*>(output.data());
        size_t available_out = output.size();
        while (!BrotliEncoderIsFinished(state)) {
            if (BrotliEncoderCompressStream(state, BROTLI_OPERATION_FINISH, &available_in, &next_in, &available_out, &next_out, nullptr) == BROTLI_FALSE)
                throw BrotliError{fmt::format("BrotliCompress failed")};
            if (available_out == 0) {
                const auto used = output.size();
                output.resize(used * 2, 0);
                next_out = reinterpret_cast<uint8_t*>(output.data() + used);
                available_out = output.size() - used;
            }
        }
        BrotliEncoderDestroyInstance(state);
        output.resize(output.size() - available_out);
        return output;
    }

      // ----------------------------------------------------------------------

    inline std::string brotli_decompress(std::string_view input, bool check_if_compressed = false)
    {
        std::string output;
        BrotliDecoderState* state = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
        BrotliDecoderResult result = BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT;
        const uint8_t* next_in = reinterpret_cast<const uint8_t*>(input.data());
        // const auto* next_in = input.data();
        size_t available_in = input.size();
        while (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
            size_t available_out = 0;
            result = BrotliDecoderDecompressStream(state, &available_in, &next_in, &available_out, nullptr, nullptr);
            const uint8_t* next_out = BrotliDecoderTakeOutput(state, &available_out);
            // fmt::print(">>>> result: {} available_out: {}\n", result, available_out);
            if (available_out != 0)
                output.insert(output.end(), next_out, next_out + available_out);
        }
        BrotliDecoderDestroyInstance(state);
        if (check_if_compressed && ((result == BROTLI_DECODER_RESULT_SUCCESS && !output.empty()) || result == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT))
            return output;
        else if (result == BROTLI_DECODER_RESULT_SUCCESS && !available_in)
            return output;
        throw BrotliError{fmt::format("BrotliDecompress failed: {}", result)};
    }

    // ----------------------------------------------------------------------

    inline bool brotli_compressed(std::string_view input)
    {
        try {
            brotli_decompress(input.substr(0, 1000), true);
            return true;
        }
        catch (BrotliError&) {
            return false;
        }
    }

} // namespace acmacs::file

// ----------------------------------------------------------------------
