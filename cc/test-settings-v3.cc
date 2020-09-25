#include "acmacs-base/settings-v3-test.hh"

// ----------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc != 3) {
        AD_ERROR("Usage: {} <file.json> <name-to-apply>", argv[0]);
        return 1;
    }

    acmacs::settings::v3::TestData data;
    data.load(argv[1]);
    data.apply(argv[2]);

    return 0;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
