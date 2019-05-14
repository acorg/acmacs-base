#include <iostream>

#include "acmacs-base/virus-name.hh"

// ----------------------------------------------------------------------

int main(int argc, const char* const* argv)
{
    int exit_code = 0;
    try {
        if (argc < 2)
            throw std::runtime_error(string::concat("Usage: ", argv[0], " <virus-name> ..."));
        for (int arg = 1; arg < argc; ++arg) {
            virus_name::Name fields(argv[arg]);
            std::cout << "SRC: " << argv[arg] << '\n'
                      << "VT:  " << fields.virus_type << '\n'
                      << "HST: " << fields.host << '\n'
                      << "LOC: " << fields.location << '\n'
                      << "ISO: " << fields.isolation << '\n'
                      << "YEA: " << fields.year << '\n'
                      << "REA: " << fields.reassortant << '\n'
                      << "EXT: " << fields.extra << '\n'
                      << '\n';
        }
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
