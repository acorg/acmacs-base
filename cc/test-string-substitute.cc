#include "acmacs-base/string-substitute.hh"

// ----------------------------------------------------------------------

int main()
{
    fmt::print("{}\n", acmacs::string::substitute("{a} {b} {{liter}} {no-value} {c}", std::pair{"a", "aaa"}, std::pair{"b", 67}, std::pair{"c", 2020.202}));
    return 0;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
