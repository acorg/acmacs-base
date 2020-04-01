#include <vector>

#include "acmacs-base/debug.hh"
#include "acmacs-base/to-string.hh"

// ----------------------------------------------------------------------

inline static void my_assert(bool condition, std::string message)
{
    if (!condition)
        throw std::runtime_error("assertion failed: " + message);
}

// ----------------------------------------------------------------------

int main(int, const char* const* argv)
{
    int exit_code = 0;
    try {
        std::vector<std::pair<double, const char*>> data{{0.1, "0.1"}, {0.2, "0.2"}, {0.3, "0.3"}, {0.4, "0.4"}, {0.5, "0.5"}, {0.6, "0.6"}, {0.7, "0.7"}, {0.8, "0.8"}, {0.9, "0.9"}, {0.91, "0.91"}, {0.912, "0.912"}, {0.9129, "0.9129"}, {1.0 / 3.1, "0.32258064516129031"}};
        for (auto [source, expected] : data) {
            const auto result = acmacs::format_double(source);
            my_assert(result == expected, argv[0] + std::string(" expected:") + expected + " got:" + result);
        }
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
