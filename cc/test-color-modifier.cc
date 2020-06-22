#include <tuple>
#include <array>

#include "acmacs-base/color-modifier.hh"
#include "acmacs-base/color-hsv.hh"

// ----------------------------------------------------------------------

using en = std::tuple<Color, std::string_view, Color>;
using namespace std::string_view_literals;

static const std::array test_data{
    en{GREEN, ":s-0.5", 0x7fff7f},
    en{GREEN, ":s-0.9", 0xE5FFE5},
    en{TRANSPARENT, ":s-0.5", TRANSPARENT},
    en{GREEN, ":s0.5", GREEN},
    en{GREEN, ":s+0.5", GREEN},
    en{0x6495ED, ":s=0.5", 0x78aaf1},

    en{BLUE, ":b-0.5", 0x00007F},
    en{BLUE, ":b-0.9", 0x000019},
    en{TRANSPARENT, ":b-0.5", TRANSPARENT},
    en{BLUE, ":b0.5", BLUE},
    en{BLUE, ":b+0.5", BLUE},

    en{0xAB3838, ":s-0.5", 0xAB7171},
    en{0xAB3838, ":s-0.9", 0xAB9F9F},
    en{0xAB3838, ":s+0.5", 0xAB1B1B},
    en{0xAB3838, ":s0.9",  0xAB0505},
    en{0x80AB3838, ":s0.9",  0x80AB0505},

    en{0xAB3838, ":b-0.5", 0x551B1B},
    en{0xAB3838, ":b-0.9", 0x110505},
    en{0xAB3838, ":b+0.5", 0xD44545},
    en{0x77AB3838, ":b+0.5", 0x77D44545},
    en{0xAB3838, ":b0.9",  0xF65050},

    en{0xAB3838, ":t-0.1",  0xAB3838},
    en{0xAB3838, ":t0.1",  0x19AB3838},

    en{TRANSPARENT, ":s+0.5", TRANSPARENT},
    en{TRANSPARENT, ":b+0.5", 0xFF7F7F7F},
    en{TRANSPARENT, ":t+0.5", TRANSPARENT},
    en{TRANSPARENT, ":t-0.5", 0x7F000000},
    en{0xFF808080,  ":t-0.5", 0x7F808080},

    en{BLACK,  ":s-0.5", BLACK},
    en{BLACK,  ":b-0.5", BLACK},
    en{BLACK,  ":t+0.5", 0x7F000000},

    en{BLUE, ":p+0.5", 0x7F7FFF},
    en{BLACK, ":p+0.5", 0x7F7F7F},
    en{0x7F7F7F, ":p+0.5", 0xBFBFBF},
};

int main()
{
    int errors{0};
    for (const auto& [source, modifier, expected] : test_data) {
        Color color{source};
        if (acmacs::color::modify(color, modifier) != expected) {
            ++errors;
            AD_ERROR("color modifier failed: \"{}\"({}) + \"{}\" -> \"{}\"({})  expected: \"{}\"({})", source, acmacs::color::HSV{source}, modifier, color, acmacs::color::HSV{color}, expected,
                     acmacs::color::HSV{expected});
        }
    }
    return errors;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
