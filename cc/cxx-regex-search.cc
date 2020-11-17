#include "acmacs-base/argv.hh"
#include "acmacs-base/regex.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    argument<str> reg{*this, arg_name{"regex"}, mandatory};
    argument<str> data{*this, arg_name{"string-to-search-in"}, mandatory};
};

// ----------------------------------------------------------------------

int main(int argc, const char* const* argv)
{
    int exit_code = 0;
    try {
        Options opt(argc, argv);
        std::regex re{opt.reg->begin(), opt.reg->end(), acmacs::regex::icase};
        std::cmatch match;
        if (acmacs::regex::search(opt.data, match, re)) {
            fmt::print("matched at {} with length {}\n", match.position(0), match.length(0));
            size_t subm{1};
            for (const auto& submatch : match) {
                fmt::print("{:2d} len:{:3d}   \"{}\"\n", subm, submatch.length(), submatch.str());
                ++subm;
            }
        }
        else
            AD_WARNING("not matched");
    }
    catch (std::exception& err) {
        AD_ERROR("{}", err);
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
