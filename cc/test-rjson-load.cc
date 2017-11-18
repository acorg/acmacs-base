#include <iostream>
#include <utility>

#include "stream.hh"
#include "rjson.hh"
#include "timeit.hh"
#include "read-file.hh"

// ----------------------------------------------------------------------

int main(int argc, const char** argv)
{
    int exit_code = 0;
    if (argc == 2) {
        try {
            Timeit ti_read(std::string{"reading from "} + argv[1] + " ");
            const std::string source = acmacs::file::read(argv[1], true);
            ti_read.report();
            Timeit ti_parse(std::string{"parsing json "});
            const auto data = rjson::parse_string(source);
            ti_parse.report();
        }
        catch (std::exception& err) {
            std::cerr << "ERROR: " << err.what() << '\n';
            exit_code = 2;
        }
    }
    else {
        std::cerr << "Usage: " << argv[0] << " <file.json[.xz]>\n";
        exit_code = 1;
    }
    return exit_code;

}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
