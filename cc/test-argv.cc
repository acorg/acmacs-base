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
