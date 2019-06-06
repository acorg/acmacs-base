#include <iostream>
#include "acmacs-base/read-file.hh"

int main()
{
    std::string data = acmacs::file::read("/home/eu/AD/data/locationdb.json.xz");
    std::cerr << data.substr(0, 20) << '\n';
    return 0;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
