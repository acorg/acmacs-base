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
        throw std::runtime_error("ACMACSD_ROOT env is not set");
    }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
