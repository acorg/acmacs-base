#pragma once

#include <exception>
#include <regex>

// ----------------------------------------------------------------------

namespace virus_name
{
    namespace _internal
    {
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

        const std::regex cdc{"^([A-Z][A-Z][A-Z]?) "};

          // [1] - type+subtype, [2] - host, [3] - location, [4] - isolation-number (omitting leading zeros), [5] - century, [6] - year (2 last digit), [7] - reassortant and passage
        const std::regex international{"^([AB][^/]*)/(?:([^/]+)/)?([^/]+)/0*([^/]+)/(19|20)?(\\d\\d)(?:(?:\\s+|__)(.+))?$"};

        inline std::string make_year(const std::smatch& m)
        {
            std::string year;
            if (m[5].length())
                year = m[5].str() + m[6].str();
            else if (m[6].str()[0] > '2')
                year = "19" + m[6].str();
            else
                year = "20" + m[6].str();
            return year;
        }

#pragma GCC diagnostic pop
    }

// ----------------------------------------------------------------------

    class Unrecognized : public std::runtime_error { public: using std::runtime_error::runtime_error; };

// ----------------------------------------------------------------------

    std::string normalize(std::string name);

// ----------------------------------------------------------------------

    std::string normalize(std::string name);

      // returned cdc abbreviation starts with #
    inline std::string location(std::string name)
    {
        using namespace _internal;
        std::string location;
        std::smatch m;
        if (std::regex_search(name, m, cdc))
            location = "#" + m[1].str();
        else if (std::regex_match(name, m, international))
            location = m[3].str();
        else
            throw Unrecognized{"No location in " + name};
        return location;
    }

// ----------------------------------------------------------------------

    inline std::string virus_type(std::string name)
    {
        using namespace _internal;
        std::string virus_type;
        std::smatch m;
        if (std::regex_match(name, m, international))
            virus_type = m[1].str();
        else
            throw Unrecognized{"No virus_type in " + name};
        return virus_type;

    } // AntigenSerum::virus_type

// ----------------------------------------------------------------------

    inline std::string year(std::string name)
    {
        using namespace _internal;
        std::string year;
        std::smatch m;
        if (std::regex_match(name, m, international))
            year = make_year(m);
        else
            throw Unrecognized{"No year in " + name};
        return year;

    } // AntigenSerum::year

// ----------------------------------------------------------------------

    inline void split(std::string name, std::string& virus_type, std::string& host, std::string& location, std::string& isolation, std::string& year, std::string& passage)
    {
        using namespace _internal;
        std::smatch m;
        if (std::regex_match(name, m, international)) {
            virus_type = m[1].str();
            host = m[2].str();
            location = m[3].str();
            isolation = m[4].str();
            year = make_year(m);
            passage = m[7].str();
        }
        else
            throw Unrecognized{"Cannot split " + name};
    }

}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
