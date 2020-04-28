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
    pp{R"([38])"sv, R"([38])"sv},
};

int main()
{
    int exit_code = 0;
    for (const auto& [to_parse, expected] : data) {
        const auto val = rjson::v3::parse_string(to_parse);
        const auto formatted = rjson::v3::format(val);
        if (formatted != expected) {
            AD_ERROR("rjson::v3 parsing/formatting failed: \"{}\" <- \"{}\", expected: \"{}\"", formatted, to_parse, expected);
            ++exit_code;
        }

        // fmt::print("rjson::v3: {} <- \"{}\"\n", rjson::v3::format(val), elt);

        // try {
        //     fmt::print("val.to<string>: \"{}\"\n", val.to<std::string>());
        // }
        // catch (std::exception& err) {
        //     fmt::print(stderr, "> ERROR {}\n", err);
        //     ++exit_code;
        // }
        // try {
        //     fmt::print("val.to<bool>: {}\n", val.to<bool>());
        // }
        // catch (std::exception& err) {
        //     fmt::print(stderr, "> ERROR {}\n", err);
        //     ++exit_code;
        // }
        // try {
        //     fmt::print("val.to<int>: {}\n", val.to<int>());
        // }
        // catch (std::exception& err) {
        //     fmt::print(stderr, "> ERROR {}\n", err);
        //     ++exit_code;
        // }
        // try {
        //     fmt::print("val.to<double>: {}\n", val.to<double>());
        // }
        // catch (std::exception& err) {
        //     fmt::print(stderr, "> ERROR {}\n", err);
        //     ++exit_code;
        // }

        // if (val.is_array()) {
        //     fmt::print("array:");
        //     for (const auto& arr_value : val.array())
        //         fmt::print(" {}", arr_value.actual_type());
        //     fmt::print("\n");
        // }
        // else if (val.is_object()) {
        //     fmt::print("object:");
        //     for (const auto& [obj_key, obj_value] : val.object())
        //         fmt::print(" \"{}\":{}", obj_key, obj_value.actual_type());
        //     fmt::print("\n");
        // }
    }
    return 0; // exit_code;
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
