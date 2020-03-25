#include "acmacs-base/argv.hh"
#include "acmacs-base/color-gradient.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    argument<str> color1{*this, arg_name{"color1"}, mandatory};
    argument<str> color2{*this, arg_name{"color2"}, mandatory};
    argument<str> color3{*this, arg_name{"color3"}, mandatory};
    argument<size_t> output_size{*this, arg_name{"output-size"}, mandatory};
};

int main(int argc, const char* argv[])
{
    int exit_code = 0;

    try {
        Options opt(argc, argv);
        const Color c1{*opt.color1}, c2{*opt.color2}, c3{*opt.color3};
        const auto result = acmacs::color::bezier_gradient(c1, c2, c3, opt.output_size);
        for (const auto& color : result)
            fmt::print("{:X}\n", color);
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
        exit_code = 1;
    }

    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
