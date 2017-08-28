#include <iostream>

#include "rjson.hh"

// ----------------------------------------------------------------------

static const char* sSource[] = {
    R"(
"Altogether elsewhere, \"vast\"")",
    R"(  2017)",
    R"(  -2017)",
    R"(  +2001.2017)",
    R"(  -2002.2017E2)",
    R"(  -2003.2017E+3)",
    R"(  -2004.2017E-4)",
    R"(  .2005)",
    R"(  -.2006)",
};

// ----------------------------------------------------------------------

int main()
{
    for (auto src: sSource) {
        std::cout << '%' << src << "%\n";
        try {
            auto val = rjson::parse(src);
            std::cout << "RESULT: " << val.to_string() << "\n\n";
        }
        catch (rjson::Error& err) {
            std::cerr << "ERROR: " << err.what() << '\n';
        }
    }
    return 0;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
