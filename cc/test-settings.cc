#include "acmacs-base/settings.hh"

// ----------------------------------------------------------------------

int main()
{
    using namespace std::string_view_literals;
    int exit_code = 0;

    try {
        {
            acmacs::settings::Settings s1({"test-settings-v2.1.json"});
            s1.apply("main"sv, acmacs::verbose::no);
        }
        {
            acmacs::settings::Settings s2({"test-settings-v2.2.json", "test-settings-v2.3.json"});
            s2.apply("main"sv, acmacs::verbose::no);
        }
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
