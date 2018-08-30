#include <iostream>

#include "rjson2.hh"
namespace rjson = rjson2;

int main(int argc, const char* const* argv)
{
    int exit_code = 0;
    if (argc == 2) {
        try {
            auto data = rjson::parse_file(argv[1]);
            std::cout << rjson::pretty(data, 2, rjson::json_pp_emacs_indent::yes) << '\n';
        }
        catch (std::exception& err) {
            std::cerr << "ERROR: " << err.what() << '\n';
        }
    }
    else {
        std::cerr << "Usage: " << argv[0] << " <source.json>\n";
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
