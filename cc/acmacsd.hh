#pragma once

#include <string>
#include <stdexcept>
#include <cstdlib>

// ----------------------------------------------------------------------

namespace acmacs
{
    inline std::string acmacsd_root()
    {
        using namespace std::string_literals;
        if (auto root = std::getenv("ACMACSD_ROOT"); root)
            return root;
        if (auto r_home = std::getenv("R_HOME"); r_home) // running under R
            return r_home + std::string("library/acmacs.r");
        throw std::runtime_error("ACMACSD_ROOT env is not set (for apache-mod-acmacs see /etc/apache2/envvars)");
    }

    inline std::string hidb_v5_dir()
    {
        using namespace std::string_literals;
        if (auto dir = std::getenv("HIDB_V5"); dir)
            return dir;
        throw std::runtime_error("HIDB_V5 env is not set (for apache-mod-acmacs see /etc/apache2/envvars)");
    }

    inline std::string locdb_v2()
    {
        using namespace std::string_literals;
        if (auto dir = std::getenv("LOCATIONDB_V2"); dir)
            return dir;
        throw std::runtime_error("LOCATIONDB_V2 env is not set (for apache-mod-acmacs see /etc/apache2/envvars)");
    }

    inline std::string seqdb_v3_dir()
    {
        using namespace std::string_literals;
        if (auto dir = std::getenv("SEQDB_V3"); dir)
            return dir;
        throw std::runtime_error("SEQDB_V3 env is not set (for apache-mod-acmacs see /etc/apache2/envvars)");
    }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
