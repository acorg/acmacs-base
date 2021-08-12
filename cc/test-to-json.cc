#include "acmacs-base/to-json.hh"

// ----------------------------------------------------------------------

int main()
{
    {
        auto js = to_json::v2::object();
        js << to_json::v2::key_val("S1", "A") << to_json::v2::key_val("N1", 7.3);
        fmt::print(fmt::runtime("{:6}\n{:4c}\n\n"), js, js);
    }

    {
        auto js = to_json::v2::object();
        fmt::print("{}\n\n", js);
        js << to_json::v2::key_val{"A1", to_json::v2::array(1, 2)}
           << to_json::v2::key_val{"A2", to_json::v2::array()}
           << to_json::v2::key_val{"O1", to_json::v2::object(
                to_json::v2::key_val{"A3", to_json::v2::array(17.21, 21.17)},
                to_json::v2::key_val{"S3", "CTAG"}
                               )}
           ;
        fmt::print("{}\n\n", js);
    }

    return 0;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
