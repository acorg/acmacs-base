// #if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L && defined(__cpp_concepts) && __cpp_concepts >= 201907L

#include <array>
#include "acmacs-base/rjson-v3.hh"

using namespace std::string_view_literals;

using pp = std::pair<std::string_view, std::string_view>;
const std::array data{
    pp{R"(  "coronavirus"  )"sv, R"("coronavirus")"sv},
    pp{R"(  2020)"sv, R"(2020)"sv},
    pp{R"(  2020.5)"sv, R"(2020.5)"sv},
    pp{R"(2.71828)"sv, R"(2.71828000000000003)"sv},
    pp{R"(  true)"sv, R"(true)"sv},
    pp{R"(  false)"sv, R"(false)"sv},
    pp{R"(  null)"sv, R"(null)"sv},
    pp{R"( {"array":[ 1   ,   2   ,  true  , false, null, ["log"], { "a" : null}  ], "another":[], "a": "a", "b": 7, "c": null, "d": true, "e": false, "f": "", "g": 3.1415 }  )"sv, R"({"array": [1, 2, true, false, null, ["log"], {"a": null}], "another": [], "a": "a", "b": 7, "c": null, "d": true, "e": false, "f": "", "g": 3.14150000000000018})"sv},
    pp{R"([38, 39, "40", false, null, {"a": true}])"sv, R"([38, 39, "40", false, null, {"a": true}])"sv},
};

int main()
{
    int exit_code = 0;
    for (const auto& [to_parse, expected] : data) {
        const auto val = rjson::v3::parse_string(to_parse);
        const auto formatted = rjson::v3::format(val, rjson::v3::output::compact_with_spaces);
        if (formatted != expected) {
            AD_ERROR("rjson::v3 parsing/formatting failed: \"{}\" <- \"{}\", expected: \"{}\"", formatted, to_parse, expected);
            ++exit_code;
        }
    }
    return exit_code;
}

// #else

// #include "acmacs-base/fmt.hh"

// int main()
// {
//     fmt::print(stderr, "warning: test-rjson-v3 not running, neither starship nor concepts supported by the compiler\n");
//     return 0;
// }
// #endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
