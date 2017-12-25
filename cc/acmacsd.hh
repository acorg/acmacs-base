#pragma once

#include <string>
#include <cstdlib>

// ----------------------------------------------------------------------

namespace acmacs
{
    inline std::string acmacsd_root()
    {
        using namespace std::string_literals;
        if (auto root = std::getenv("ACMACSD_ROOT"); root)
            return root;
        return std::getenv("HOME") + "/AD"s;
    }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
