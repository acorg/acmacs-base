#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L && defined(__cpp_concepts) && __cpp_concepts >= 201907L

#include "acmacs-base/rjson-v3.hh"

int main()
{
    int exit_code = 0;
    const auto val = rjson::v3::parse_string(R"( {"array":[ 1   ,   2   ,  true  , { "a" : null}  ], "another":[], "a": "a", "b": 7, "c": null, "d": true, "e": false, "f": "", "g": 3.1415 }  )");
    return exit_code;
}

#else

#include "acmacs-base/fmt.hh"

int main()
{
    fmt::print(stderr, "warning: test-rjson-v3 not running, neither starship nor concepts supported by the compiler\n");
    return 0;
}
#endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
