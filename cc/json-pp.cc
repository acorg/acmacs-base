#include "acmacs-base/rjson-v3.hh"
#include "acmacs-base/timeit.hh"

// ----------------------------------------------------------------------

int main(int argc, const char* const* argv)
{
    int exit_code = 0;
    if (argc == 2) {
        try {
            Timeit parsing_timer("rjson::v3 parsing: ");
            const auto data = rjson::v3::parse_file(argv[1]);
            parsing_timer.report();

            Timeit printing_timer("rjson::v3 printing: ");
            fmt::print("{}\n", data);
        }
        catch (std::exception& err) {
            fmt::print(stderr, "> ERROR {}\n", err);
            exit_code = 2;
        }
    }
    else {
        fmt::print(stderr, "Usage: {} <source.json>\n", argv[0]);
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
