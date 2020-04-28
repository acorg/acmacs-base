#include "acmacs-base/rjson-v2.hh"
#include "acmacs-base/timeit.hh"

// ----------------------------------------------------------------------

int main(int argc, const char* const* argv)
{
    int exit_code = 0;
    if (argc == 2) {
        try {
            Timeit parsing_timer("rjson::v2 parsing: ");
            auto data = rjson::v2::parse_file(argv[1]);
            parsing_timer.report();

            Timeit printing_timer("rjson::v2 printing: ");
            fmt::print("{}\n", rjson::v2::pretty(data));
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
