#include "acmacs-base/argv.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;

struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<bool> a{*this, 'a', "a-enable"};
    option<bool> b{*this, 'b'};
    option<bool> c{*this, "c-enable"};
    option<str> d{*this, 'd', "ds", desc{"DS"}};
    option<str> e{*this, 'e', "ee", dflt{"ee-ee"}, arg_name{"E_ARG"}, desc{"EE"}};
    option<str> m{*this, 'm', "mm", mandatory, desc{"MM"}};
    option<int> i{*this, 'i', "ii", dflt{-1167}, desc{"int"}};
    option<size_t> s{*this, 's', "size", dflt{1267UL}, desc{"size_t"}};
    option<double> f{*this, 'f', "float", dflt{13.67}, desc{"float"}};

    argument<str> source{*this, arg_name{"source"}, mandatory};
    argument<str> output{*this, arg_name{"output"}, dflt{""}};
};

// ----------------------------------------------------------------------

int main(int argc, const char* const argv[])
{
    int exit_code = 0;
    try {
        Options opt(argc, argv);
        fmt::print("a: {}\nb: {}\nc: {}\nd: {}\ne: {}\nm: {}\ni: {}\ns: {}\nf: {}\nsource: {}\noutput: {}\n", opt.a, opt.b, opt.c, opt.d, opt.e, opt.m, opt.i, opt.s, opt.f, opt.source, opt.output);
    }
    catch (std::exception& err) {
        fmt::print(stderr, "> ERROR {}\n", err);
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
