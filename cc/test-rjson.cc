#include <iostream>

#include "rjson.hh"

// ----------------------------------------------------------------------

static const char* sSource[] = {
    R"(
"Altogether elsewhere, \"vast\"")",
    R"(  2017)",
    R"(  -2017)",
    R"(  2017.2017)",
    R"(  -2017.2017E2)",
    R"(  -2017.2017E-2)",
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
