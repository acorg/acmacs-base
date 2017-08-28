#include <iostream>
#include <utility>

#include "rjson.hh"

// ----------------------------------------------------------------------

static const std::pair<const char*, const char*> sSource[] = {
    {R"(
          "Altogether elsewhere, \"vast\""  )", R"("Altogether elsewhere, \"vast\"")"},
    {R"(  2017)", R"(2017)"},
    {R"(  -2017)", R"(-2017)"},
    {R"(  +2001.21)", R"(2001.2100000000000363797880709171)"},
    {R"(  -2002.22E2)", R"(-200222)"},
    {R"(  -2003.3E+3)", R"(-2003300)"},
    {R"(  -2004.4E-4)", R"(-0.20044000000000000705213665241899)"},
    {R"(  .2005)", R"(0.20050000000000001154631945610163)"},
    {R"(  -.2006)", R"(-0.20060000000000000053290705182008)"},
};

// ----------------------------------------------------------------------

int main()
{
    int exit_code = 0;


    for (auto [to_parse, expected]: sSource) {
        try {
              // std::cout << '%' << to_parse << "%\n";
            const auto val = rjson::parse(to_parse);
            const auto result = val.to_string();
            if (result != expected) {
                std::cerr << "ERROR: parsing %" << to_parse << "%\n  --> %" << result << "%\n  expected: %" << expected << "%\n\n";
                exit_code = 1;
            }
        }
        catch (rjson::Error& err) {
            std::cerr << "ERROR: parsing %" << to_parse << "%\n  --> " << err.what() << "\n  expected: %" << expected << "%\n\n";
            exit_code = 2;
        }
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
