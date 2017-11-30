#include <iostream>
#include <cassert>
#include <cstring>

#include "stream.hh"
#include "float.hh"
#include "string-split.hh"

// ----------------------------------------------------------------------

int main()
{
    int exit_code = 0;
    try {
        const std::string S1{"1,2,,3"};
        const std::string_view SV1(S1);
        const std::string S2{"1,2,3,5,8"};
        const std::string S3{"1,-2,3.5,8.8,999e21"};

        {
            auto r = acmacs::string::split(S1, ",");
            assert(r.size() == 4);
            assert(r[0] == "1");
            assert(r[1] == "2");
            assert(r[2].empty());
            assert(r[3] == "3");
        }

        {
            auto r = acmacs::string::split(S1, ",", acmacs::string::Split::RemoveEmpty);
            assert(r.size() == 3);
            assert(r[0] == "1");
            assert(r[1] == "2");
            assert(r[2] == "3");
        }

        {
            auto r = acmacs::string::split(SV1, ",");
            assert(r.size() == 4);
            assert(r[0] == "1");
            assert(r[1] == "2");
            assert(r[2].empty());
            assert(r[3] == "3");
        }

        {
            auto r = acmacs::string::split(SV1, ",", acmacs::string::Split::RemoveEmpty);
            assert(r.size() == 3);
            assert(r[0] == "1");
            assert(r[1] == "2");
            assert(r[2] == "3");
        }

        {
            auto r = acmacs::string::split_into_uint(S2, ",");
            assert(r.size() == 5);
            assert((r == std::vector<size_t>{1,2,3,5,8}));
        }

        {
            auto r = acmacs::string::split_into_double(S3, ",");
            assert(r.size() == 5);
            // assert(float_equal(r[0], 1.0));
            // assert(float_equal(r[1], -2.0));
            assert((r == std::vector<double>{1,-2,3.5,8.8,999e21}));
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
