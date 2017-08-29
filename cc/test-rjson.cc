#include <iostream>
#include <utility>

#include "rjson.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

static const std::pair<const char*, std::variant<const char*, rjson::Error>> sSource[] = {
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
    {R"( true)", R"(true)"},
    {R"( false)", R"(false)"},
    {R"(null)", R"(null)"},
    {R"( { "a" : "b"}  )", R"({"a":"b"})"},
    {R"({x"a" : "b"}  )", rjson::Error(1, 2, "unexpected symbol: x (0x78)")},
    {R"({"a","b"})", rjson::Error(1, 5, "unexpected comma, colon is expected there")},
    {R"({"a":,"b"})", rjson::Error(1, 6, "unexpected symbol: , (0x2C)")},
    {R"({,})", rjson::Error(1, 2, "unexpected comma right after the beginning of an object")},
    {R"( { "a" :   1234   }  )", R"({"a":1234})"},
    {R"( { "a" : -1234}  )", R"({"a":-1234})"},
    {R"( { "a" : 12.34}  )", R"({"a":12.33999999999999985789145284798})"},
    {R"( { "a" : true}  )", R"({"a":true})"},
    {R"( { "a" : false}  )", R"({"a":false})"},
    {R"( { "a" : null}  )", R"({"a":null})"},
    {R"( { "a" : null,}  )", rjson::Error(1, 15, "unexpected } -- did you forget to remove last comma?")},
    {R"( { "a" : null "b": false}  )", rjson::Error(1, 15, "unexpected \" -- did you forget comma?")},
    {R"( { "a" : null  ,  ,  "b": false}  )", rjson::Error(1, 19, "unexpected comma -- two successive commas?")},
    {R"( { "a" : {"b" : 1  , "c"  : "c"  , "sub":{"xsub": false},"d": null  }}  )", R"({"a":{"b":1,"c":"c","sub":{"xsub":false},"d":null}})"},
    {R"([])", R"([])"},
    {R"([  ,  ])", rjson::Error(1, 4, "unexpected symbol: , (0x2C)")},
    {R"(["a"])", R"(["a"])"},
    {R"(["a",1])", R"(["a",1])"},
    {R"([true,false,null,"aaa"])", R"([true,false,null,"aaa"])"},
    {R"( [ 1   ,   2   ,  true  , { "a" : null}  ]  )", R"([1,2,true,{"a":null}])"},
    {R"( {"array":[ 1   ,   2   ,  true  , { "a" : null}  ], "another":[]}  )", R"([1,2,true,{"a":null}])"},
};

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

int main()
{
    int exit_code = 0;

    for (auto [to_parse, expected]: sSource) {
        const auto& to_parse_ref = to_parse; // to capture by lambda below, cannot capture to_parse (bug in llvm 4.0.1?)
        try {
              // std::cout << '%' << to_parse << "%\n";
            const auto val = rjson::parse(to_parse);
            const auto result = val.to_string();
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
        catch (rjson::Error& err) {
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
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
