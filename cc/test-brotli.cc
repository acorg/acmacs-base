#include "acmacs-base/brotli.hh"
#include "acmacs-base/argv.hh"
#include "acmacs-base/read-file.hh"

using namespace acmacs::argv;

struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    argument<str> source{*this, arg_name{"source"}, mandatory};
    argument<str> output{*this, arg_name{"output"}, dflt{""}};
};

// ----------------------------------------------------------------------

int main(int argc, const char* const argv[])
{
    int exit_code = 0;
    try {
        Options opt(argc, argv);
        acmacs::file::read_access raw_data(opt.source);
        const auto data = raw_data.raw();
        fmt::print(stderr, ">>>> {} -> {}\n", opt.source, data.size());
        if (acmacs::file::brotli_compressed(data)) {
            fmt::print(stderr, ">>>> decompressing\n");
            const std::string decompressed = acmacs::file::brotli_decompress(data);
            fmt::print(stderr, ">>>> decompressed: {} ratio: {:.4f}\n", decompressed.size(), static_cast<double>(data.size()) / static_cast<double>(decompressed.size()));
        }
        else {
            fmt::print(stderr, ">>>> compressing\n");
            const std::string compressed = acmacs::file::brotli_compress(data);
            fmt::print(stderr, ">>>> compressed: {} ratio: {:.4f}\n", compressed.size(), static_cast<double>(compressed.size()) / static_cast<double>(data.size()));
        }
    }
    catch (std::exception& err) {
        fmt::print(stderr, "> ERROR {}\n", err);
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
