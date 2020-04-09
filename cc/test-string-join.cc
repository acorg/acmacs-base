#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>

#include "acmacs-base/string-join.hh"

// ----------------------------------------------------------------------

int main(int /*argc*/, const char* /*argv*/[])
{
    using namespace std::string_literals;
    using namespace std::string_view_literals;
    int exit_code = 0;

    try {
        assert(acmacs::string::join("/", "A", std::string_view{}) == "A"s);
        assert(acmacs::string::join("/", "A"sv, std::string_view{}) == "A"s);
        assert(acmacs::string::join("/", "A"s, std::string_view{}) == "A"s);
        assert(acmacs::string::join("/", "A"s, "B"s) == "A/B"s);
        assert(acmacs::string::join("/", "A", "B"s) == "A/B"s);
        // fmt::print(stderr, "{}\n", acmacs::string::join("/", "A", "B"));
        assert(acmacs::string::join("/", "A", "B") == "A/B"s);

        assert(acmacs::string::join("/", "A", 3) == "A/3"s);
        assert(acmacs::string::join("/", "A"s, 3) == "A/3"s);
        assert(acmacs::string::join("/", "A"sv, 3) == "A/3"s);

        // fmt::print(stderr, "{}\n", acmacs::string::join("/", "A"sv, 3, "B", "", 0.5, "C"));
        assert(acmacs::string::join("/", "A"sv, 3, "B", "", 0.5, "C") == "A/3/B/0.5/C"s);
    }
    catch (std::exception& err) {
        AD_ERROR("{}", err);
        exit_code = 1;
    }

    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
