#include "acmacs-base/to-json.hh"

// ----------------------------------------------------------------------

int main()
{
    auto js = to_json::object();
    js << to_json::key_val{"ARR2", to_json::array(1, 2)}
    << to_json::key_val{"ARR3", to_json::array(1, 2, "x")}
    << to_json::key_val{"OBJ1", to_json::object(to_json::key_val{"s1", to_json::value("s1")}, to_json::key_val{"s2", to_json::value("s2")})}
    ;
    // 11.2)} << to_json::key_val{"aa", to_json::value("xx")};
    fmt::print("{}\n", js);
    // to_json::append(js, to_json::value("jopa"));
    // to_json::append(js, to_json::value("jopa"));
    // to_json::append(js, to_json::key_val{"jopa", to_json::value(11.2)});
    // fmt::print("{}\n", js);
    return 0;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
