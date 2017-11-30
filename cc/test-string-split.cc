#include <iostream>
#include <cassert>
#include <cstring>

#include "stream.hh"
#include "string-split.hh"

// ----------------------------------------------------------------------

int main()
{
    int exit_code = 0;
    try {
        const std::string S1{"1,2,,3"};
        const std::string_view SV1(S1);

        {
            auto r = acmacs::string::split(S1, ",");
            assert(r.size() == 4);
            assert(r[0] == "1");
            assert(r[1] == "2");
            assert(r[2].empty());
            assert(r[3] == "3");
              // std::cout << r1 << '\n';
        }

        {
            auto r = acmacs::string::split(S1, ",", acmacs::string::Split::RemoveEmpty);
            assert(r.size() == 3);
            assert(r[0] == "1");
            assert(r[1] == "2");
            assert(r[2] == "3");
              // std::cout << r2 << '\n';
        }

        {
            auto r = acmacs::string::split(SV1, ",");
            assert(r.size() == 4);
            assert(r[0] == "1");
            assert(r[1] == "2");
            assert(r[2].empty());
            assert(r[3] == "3");
              // std::cout << r1 << '\n';
        }

        {
            auto r = acmacs::string::split(SV1, ",", acmacs::string::Split::RemoveEmpty);
            assert(r.size() == 3);
            assert(r[0] == "1");
            assert(r[1] == "2");
            assert(r[2] == "3");
              // std::cout << r2 << '\n';
        }

    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
