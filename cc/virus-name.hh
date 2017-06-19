#pragma once

#include <exception>
#include <regex>
#include <iostream>

#include "acmacs-base/config.hh"

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

        constexpr const char* re_international_name = "^(?:([AB][^/]*)/)?(?:([^/]+)/)?([^/]+)/0*([^/]+)/(19|20)?(\\d\\d)";
        constexpr const char* re_reassortant_passage = "(?:(?:\\s+|__)(.+))?";

          // [1] - type+subtype, [2] - host, [3] - location, [4] - isolation-number (omitting leading zeros), [5] - century, [6] - year (2 last digit), [7] - reassortant and passage
          // const std::regex international{"^([AB][^/]*)/(?:([^/]+)/)?([^/]+)/0*([^/]+)/(19|20)?(\\d\\d)(?:(?:\\s+|__)(.+))?$"};
        const std::regex international{std::string(re_international_name) + re_reassortant_passage + "$"};
        const std::regex international_name{re_international_name};

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
            THROW(Unrecognized{"No location in " + name}, std::string{});
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
            THROW(Unrecognized{"No virus_type in " + name}, std::string{});
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
            THROW(Unrecognized{"No year in " + name}, std::string{});
        return year;

    } // AntigenSerum::year

// ----------------------------------------------------------------------

    inline void split(std::string name, std::string& virus_type, std::string& host, std::string& location, std::string& isolation, std::string& year, std::string& passage)
    {
        std::smatch m;
        if (std::regex_match(name, m, _internal::international)) {
            virus_type = m[1].str();
            host = m[2].str();
            location = m[3].str();
            isolation = m[4].str();
            year = _internal::make_year(m);
            passage = m[7].str();
        }
        else
            THROW_OR_VOID(Unrecognized{"Cannot split " + name});
    }

// ----------------------------------------------------------------------

      // Extracts virus name without passage, reassortant, extra,
      // etc. and calculates match threshold (to use with
      // acmacs_chart::Antigens::find_by_name_matching), match threshold is a square
      // of virus name length.
    inline size_t match_threshold(std::string name)
    {
        size_t result = 0;
        std::smatch m;
        if (std::regex_search(name, m, _internal::international_name)) {
              // find end of year (m[6])
            const auto end_of_year_offset = static_cast<size_t>(m[6].second - name.begin());
            result = end_of_year_offset * end_of_year_offset;
              // std::cerr << "INFO: match_threshold: end_of_year_offset:" << end_of_year_offset << " name:" << name << std::endl;
        }
        return result;
    }

// ----------------------------------------------------------------------

}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
