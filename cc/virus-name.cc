#include <iostream>

#include "acmacs-base/virus-name.hh"
#include "acmacs-base/string.hh"

// ----------------------------------------------------------------------

namespace virus_name
{
    std::string normalize(std::string name)
    {
        std::string result;
        std::string virus_type, host, location, isolation, year, passage;
        try {
            split(name, virus_type, host, location, isolation, year, passage);
            virus_type = string::strip(virus_type);
            host = string::strip(host);
            location = string::strip(location);
            isolation = string::strip(isolation);
            auto first_not_zero = isolation.find_first_not_of('0');
            if (first_not_zero != std::string::npos)
                isolation.erase(0, first_not_zero);
            year = string::strip(year);
            passage = string::strip(passage);
            if (host.empty())
                result = string::join("/", {virus_type, location, isolation, year});
            else
                result = string::join("/", {virus_type, host, location, isolation, year});
            if (!passage.empty()) {
                std::cerr << "WARNING: name contains extra: \"" << name << '"' << std::endl;
                result.append(1, ' ');
                result.append(passage);
            }
        }
        catch (Unrecognized&) {
            std::cerr << "WARNING: Virus name unrecognized: \"" << name << '"' << std::endl;
            result = name;
        }
        return result;

    } // normalize

}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
