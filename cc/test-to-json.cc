#include "acmacs-base/to-json.hh"

// ----------------------------------------------------------------------

int main()
{
    // {
    //     auto js = to_json::v1_5::object();
    //     fmt::print("{}\n\n", js);
    //     js << to_json::v1_5::key_val{"ARR2", to_json::v1_5::array(1, 2)};
    //     fmt::print("{}\n", js);
    // }

    {
        auto js = to_json::v2::object();
        js << to_json::v2::key_val("S1", "A") << to_json::v2::key_val("N1", 7.3);
        fmt::print("{}\n\n", js);
    }

    {
        auto js = to_json::v2::object();
        fmt::print("{}\n\n", js);
        js << to_json::v2::key_val{"A1", to_json::v2::array(1, 2)} << to_json::v2::key_val{"A2", to_json::v2::array()};
        fmt::print("{}\n\n", js);
    }

    // {
    //     auto js = to_json::object();
    //     js << to_json::key_val{"ARR2", to_json::array(1, 2)} << to_json::key_val{"ARR3", to_json::array(1, 2, "x")}
    //        << to_json::key_val{"OBJ1", to_json::object(to_json::key_val{"s1", to_json::value("s1")}, to_json::key_val{"s2", to_json::value("s2")})};
    //     // 11.2)} << to_json::key_val{"aa", to_json::value("xx")};
    //     fmt::print("{}\n", js);
    // }

    return 0;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
