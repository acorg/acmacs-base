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

          // [1] - host, [2] - location, [3] - isolation-number (omitting leading zeros), [4] - century, [5] - year (2 last digit), [6] - reassortant and passage
        const std::regex international{"^[AB][^/]*/(?:([^/]+)/)?([^/]+)/0*([^/]+)/(19|20)?(\\d\\d)(?:(?:\\s+|__)(.+))?$"};

        inline std::string make_year(const std::smatch& m)
        {
            std::string year;
            if (m[4].length())
                year = m[4].str() + m[5].str();
            else if (m[5].str()[0] > '2')
                year = "19" + m[5].str();
            else
                year = "20" + m[5].str();
            return year;
        }

#pragma GCC diagnostic pop
    }

// ----------------------------------------------------------------------

    class Unrecognized : public std::exception {};

// ----------------------------------------------------------------------

      // returned cdc abbreviation starts with #
    inline std::string location(std::string name)
    {
        using namespace _internal;
        std::string location;
        std::smatch m;
        if (std::regex_search(name, m, cdc))
            location = "#" + m[1].str();
        else if (std::regex_match(name, m, international))
            location = m[2].str();
        else
            throw Unrecognized{};
        return location;
    }

// ----------------------------------------------------------------------

    inline std::string year(std::string name)
    {
        using namespace _internal;
        std::string year;
        std::smatch m;
        if (std::regex_match(name, m, international))
            year = make_year(m);
        else
            throw Unrecognized{};
        return year;

    } // AntigenSerum::year

// ----------------------------------------------------------------------

    inline void split(std::string name, std::string& host, std::string& location, std::string& isolation, std::string& year, std::string& passage)
    {
        using namespace _internal;
        std::smatch m;
        if (std::regex_match(name, m, international)) {
            host = m[1].str();
            location = m[2].str();
            isolation = m[3].str();
            year = make_year(m);
            passage = m[6].str();
        }
        else
            throw Unrecognized{};
    }

}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
