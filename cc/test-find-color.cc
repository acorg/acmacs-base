#include "acmacs-base/color.hh"

// ----------------------------------------------------------------------

int main(int, const char* const* /*argv*/)
{
    int exit_code = 0;
    try {
        exit_code = acmacs::color::test_find_color_by_name();
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
