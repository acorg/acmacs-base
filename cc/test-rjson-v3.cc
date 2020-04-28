// #if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L && defined(__cpp_concepts) && __cpp_concepts >= 201907L

#include <array>
#include "acmacs-base/rjson-v3.hh"

using namespace std::string_view_literals;

const std::array data{
    R"("coronavirus")"sv,
    R"(2020)"sv,
    R"(2020.5)"sv,
    R"(true)"sv,
    R"(false)"sv,
    R"(null)"sv,
    R"( {"array":[ 1   ,   2   ,  true  , { "a" : null}  ], "another":[], "a": "a", "b": 7, "c": null, "d": true, "e": false, "f": "", "g": 3.1415 }  )"sv,
    R"([38])"sv,
};

int main()
{
    int exit_code = 0;
    for (const auto& elt : data) {
        const auto val = rjson::v3::parse_string(elt);
        fmt::print("rjson::v3: {}\n", val);
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
