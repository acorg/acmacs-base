#include <iostream>

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
        // const char* const t_argv[] = {argv[0], "-a", "A1-aa", "", "A2-bb", "-", "--", "A3-cc", "--c-enable"}; // "-h", "-d", "D-ARG"
        //   // const char* const t_argv[] = {"test-1", "-a", "--aa", "-s", "s1", "-s", "s2", "--settings=s3", "--settings", "s4", "aaa", "aaaa"};
        // Options opt(std::end(t_argv) - std::begin(t_argv), t_argv);
        // std::cout << opt << '\n';

        Options opt(argc, argv);
        std::cout << "a: " << opt.a << '\n';
        std::cout << "b: " << opt.b << '\n';
        std::cout << "c: " << opt.c << '\n';
        std::cout << "d: " << opt.d << '\n';
        std::cout << "e: " << opt.e << '\n';
        std::cout << "m: " << opt.m << '\n';
        std::cout << "i: " << opt.i << '\n';
        std::cout << "s: " << opt.s << '\n';
        std::cout << "f: " << opt.f << '\n';
        std::cout << "source: " << opt.source << '\n';
        std::cout << "output: " << opt.output << '\n';
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
