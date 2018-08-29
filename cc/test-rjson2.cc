#include <iostream>
#include <utility>

#include "acmacs-base/rjson2.hh"
#include "acmacs-base/stream.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

static const std::pair<std::vector<const char*>, std::variant<const char*, rjson2::parse_error>> sSource[] = {
    {{R"(
          "Altogether elsewhere, \"vast\""  )"}, R"("Altogether elsewhere, \"vast\"")"},
    {{R"(  2017)"}, R"(2017)"},
    {{R"(  -2017)"}, R"(-2017)"},
    {{R"(  +2001.21)"}, R"(+2001.21)"},
    {{R"(  -2002.22E2)"}, R"(-2002.22E2)"},
    {{R"(  -2003.3E+3)"}, R"(-2003.3E+3)"},
    {{R"(  -2004.4E-4)"}, R"(-2004.4E-4)"},
    {{R"(  .2005)"}, R"(.2005)"},
    {{R"(  -.2006)"}, R"(-.2006)"},
    {{R"( true)"}, R"(true)"},
    {{R"( false)"}, R"(false)"},
    {{R"(null)"}, R"(null)"},
    {{R"( { "a" : "b"}  )"}, R"({"a":"b"})"},
    {{R"({x"a" : "b"}  )"}, rjson2::parse_error(1, 2, "unexpected symbol: 'x' (0x78)")},
    {{R"({"a","b"})"}, rjson2::parse_error(1, 5, "unexpected comma, colon is expected there")},
    {{R"({"a":,"b"})"}, rjson2::parse_error(1, 6, "unexpected symbol: ',' (0x2C)")},
    {{R"({,})"}, rjson2::parse_error(1, 2, "unexpected comma right after the beginning of an object")},
    {{R"( { "a" :   1234   }  )"}, R"({"a":1234})"},
    {{R"( { "a" : -1234}  )"}, R"({"a":-1234})"},
    {{R"( { "a" : 12.34}  )"}, R"({"a":12.34})"},
    {{R"( { "a" : true}  )"}, R"({"a":true})"},
    {{R"( { "a" : false}  )"}, R"({"a":false})"},
    {{R"( { "a" : null}  )"}, R"({"a":null})"},
    {{R"( { "a" : null,}  )"}, rjson2::parse_error(1, 15, "unexpected } -- did you forget to remove last comma?")},
    {{R"( { "a" : null "b": false}  )"}, rjson2::parse_error(1, 15, "unexpected \" -- did you forget comma?")},
    {{R"( { "a" : null  ,  ,  "b": false}  )"}, rjson2::parse_error(1, 19, "unexpected comma -- two successive commas?")},
    {{R"( { "a" : {"b" : 1  , "c"  : "c"  , "sub":{"xsub": false},"d": null  }}  )"}, R"({"a":{"b":1,"c":"c","d":null,"sub":{"xsub":false}}})"},
    {{R"([])"}, R"([])"},
    {{R"([  ,  ])"}, rjson2::parse_error(1, 4, "unexpected comma right after the beginning of an array")},
    {{R"(["a"])"}, R"(["a"])"},
    {{R"(["a",1])"}, R"(["a",1])"},
    {{R"([true,false,null,"aaa"])"}, R"([true,false,null,"aaa"])"},
    {{R"( [ 1   ,   2   ,  true  , { "a" : null}  ]  )"}, R"([1,2,true,{"a":null}])"},
    {{R"( {"array":[ 1   ,   2   ,  true  , { "a" : null}  ], "another":[]}  )"}, R"({"another":[],"array":[1,2,true,{"a":null}]})"},

      // merge json objects
    {{R"({"a": "a", "b": {"ba": "ba"}})", R"({"b": {"ba": "BAAA"}, "c": "c"})"}, R"({"a":"a","b":{"ba":"BAAA"},"c":"c"})"},
    {{R"({"a": 1, "b": {"ba": 2}})", R"({"b": {"ba": 3.3}, "c": true})"}, R"({"a":1,"b":{"ba":3.3},"c":true})"},
    {{R"({"a": 1, "b": ["ba", "bb"]})", R"({"b": ["BA", 3.3], "c": true})"}, R"({"a":1,"b":["BA",3.3],"c":true})"},
};

static const char* cText = "Zugriff auf alle";
static const std::pair<rjson2::value, const char*> sSourceRJ[] = {
    {{}, "null"},
    {rjson2::null{}, "null"},
    {{"Bundesstadt"}, R"("Bundesstadt")"},
    {{std::string("Bundesstadt")}, R"("Bundesstadt")"},
    {{std::string_view(cText)}, R"("Zugriff auf alle")"},
    {{2012}, R"(2012)"},
    {{+2012}, R"(2012)"},
    {{-2012}, R"(-2012)"},
    {{2012.25}, R"(2012.25)"},
    {{+2012.5}, R"(2012.5)"},
    {{-2012.125}, R"(-2012.125)"},
    {{2012.33e2}, R"(201233)"},
};

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

int main()
{
    int exit_code = 0;

    for (auto [to_parse, expected]: sSource) {
        const auto& to_parse_ref = to_parse; // to capture by lambda below, cannot capture to_parse (bug in llvm 4.0.1?)
        try {
            // std::cerr << "DEBUG: " << to_parse << '\n';
            auto val = rjson2::parse_string(to_parse[0]);
            for (auto& to_parse_e: to_parse) {
                if (&to_parse_e != &to_parse.front())
                    val.update(rjson2::parse_string(to_parse_e));
            }

            const auto result = rjson2::to_string(val);
            // std::cerr << "DEBUG: " << to_parse << " --> " << result << '\n';
            std::visit([&](auto&& aExpected) {
                using T = std::decay_t<decltype(aExpected)>;
                if constexpr (std::is_same_v<T, const char*>) {
                    if (result != aExpected) {
                        std::cerr << "ERROR: parsing %" << to_parse_ref << "%\n  --> %" << result << "%\n  expected: %" << aExpected << "%\n\n";
                        exit_code = 1;
                    }
                }
                else {
                    std::cerr << "ERROR: parsing %" << to_parse_ref << "%\n  --> %" << result << "%\n  expected: %" << aExpected.what() << "%\n\n";
                    exit_code = 1;
                }
            }, expected);
        }
        catch (rjson2::parse_error& err) {
            std::visit([&](auto&& aExpected) {
                using T = std::decay_t<decltype(aExpected)>;
                if constexpr (std::is_same_v<T, const char*>) {
                    std::cerr << "ERROR: parsing %" << to_parse_ref << "%\n  --> " << err.what() << "\n  expected: %" << aExpected << "%\n\n";
                    exit_code = 2;
                }
                else {
                    if (std::string{err.what()} != aExpected.what()) {
                        std::cerr << "ERROR: parsing %" << to_parse_ref << "%\n  --> %" << err.what() << "%\n  expected: %" << aExpected.what() << "%\n\n";
                        exit_code = 2;
                    }
                }
            }, expected);
        }
    }

    for (const auto& [val, expected]: sSourceRJ) {
        const auto result = rjson2::to_string(val);
        // std::cerr << "DEBUG: " << result << " == " << expected << '\n';
        if (result != expected) {
            std::cerr << "ERROR: after initialization %" << result << "%  expected: %" << expected << "%\n";
            exit_code = 1;
        }
    }

    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
