#pragma once

#include <exception>
#include <regex>
#include <iostream>

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

        const std::regex cdc_name{"^([A-Z][A-Z][A-Z]?)[ \\-]"}; // cdc names had location abbreviation separated with '-' earlier

        constexpr const char* re_international_name = "^(?:([AB][^/]*)/)?(?:([^/]+)/)?([^/]{2,})/0*([^/]+)/(19|20)?(\\d\\d)";
        constexpr const char* re_reassortant_passage = "(?:(?:\\s+|__)(.+))?";

          // [1] - type+subtype, [2] - host, [3] - location, [4] - isolation-number (omitting leading zeros), [5] - century, [6] - year (2 last digit), [7] - reassortant and passage
          // const std::regex international{"^([AB][^/]*)/(?:([^/]+)/)?([^/]+)/0*([^/]+)/(19|20)?(\\d\\d)(?:(?:\\s+|__)(.+))?$"};
        const std::regex international{std::string(re_international_name) + re_reassortant_passage + "$"};
        const std::regex international_name{re_international_name};
        const std::regex international_name_with_extra{std::string(re_international_name) + "(?:\\s*(.+))?"};

        const std::regex passage_after_name{" (MDCK|SIAT|MK|E|X)[X\\?\\d]"}; // to extract cdc name only! NOT to extract passage!

#pragma GCC diagnostic pop

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

        constexpr const size_t international_name_suffix_size = 9; // len(lo/isolation-number/year) >= 9

        inline std::string location_human(std::string name, size_t prefix_size)
        {
            const auto end = name.find('/', prefix_size);
            return name.substr(prefix_size, end - prefix_size);
        }
    }

// ----------------------------------------------------------------------

    class Unrecognized : public std::runtime_error { public: using std::runtime_error::runtime_error; };

// ----------------------------------------------------------------------

    std::string normalize(std::string name);

// ----------------------------------------------------------------------

      // Extracts virus name without passage, reassortant, extra, etc.
    inline std::string_view name(std::string_view aFullName)
    {
        std::cmatch m;
        if (std::regex_search(aFullName.begin(), aFullName.end(), m, _internal::international_name)) {
            return aFullName.substr(0, static_cast<size_t>(m.length()));
        }
        else if (std::regex_search(aFullName.begin(), aFullName.end(), m, _internal::passage_after_name)) { // works for cdc name without extra and without reassortant (cdc names usually do not have reassortant)
            return aFullName.substr(0, static_cast<size_t>(m.position()));
        }
        else {
            return aFullName;   // failed to split, perhaps cdc name without passage
        }
    }

// ----------------------------------------------------------------------

    using location_func_t = std::string (*)(std::string);
    enum class prioritize_cdc_name { no, yes };

    inline std::string location_for_cdc_name(std::string name)
    {
        std::smatch m;
        if (std::regex_search(name, m, _internal::cdc_name))
            return "#" + m[1].str();
        throw Unrecognized{"No cdc abbreviation in " + name};
    }

    // returned cdc abbreviation starts with #
    inline std::string location(std::string name, prioritize_cdc_name check_cdc_first = prioritize_cdc_name::no)
    {
        try {
            if (check_cdc_first == prioritize_cdc_name::yes)
                return location_for_cdc_name(name);
        }
        catch (Unrecognized&) {
        }

        std::smatch m;
        if (std::regex_search(name, m, _internal::international_name)) // international name with possible "garbage" after year, e.g. A/TOKYO/UT-IMS2-1/2014_HY-PR8-HA-N121K
            return m[3].str();

        try {
            if (check_cdc_first == prioritize_cdc_name::no)
                return location_for_cdc_name(name);
        }
        catch (Unrecognized&) {
        }

        throw Unrecognized{"No location in " + name};
    }

    // Faster version of location() for A(H3N2)/ and A(H1N1)/ names without host field
    inline std::string location_human_a(std::string name)
    {
        constexpr const size_t prefix_size = 8;
        return (name.size() > (prefix_size + _internal::international_name_suffix_size) && name[prefix_size - 1] == '/') ? _internal::location_human(name, prefix_size) : location(name);
    }

    // Faster version of location() for B/ names without host field
    inline std::string location_human_b(std::string name)
    {
        constexpr const size_t prefix_size = 2;
        return (name.size() > (prefix_size + _internal::international_name_suffix_size) && name[prefix_size - 1] == '/') ? _internal::location_human(name, prefix_size) : location(name);
    }

    // ----------------------------------------------------------------------

    inline std::string_view virus_type(const std::string& name) // pass by reference! because we return string_view to it
    {
        std::smatch m;
        if (std::regex_search(name, m, _internal::international_name))
            return {name.data() + m.position(1), static_cast<size_t>(m.length(1))};
        throw Unrecognized("No virus_type in " + name);

    } // AntigenSerum::virus_type

    // ----------------------------------------------------------------------

    inline std::string year(std::string name)
    {
        std::smatch m;
        if (std::regex_search(name, m, _internal::international_name))
            return _internal::make_year(m);
        throw Unrecognized("No year in " + name);

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
            throw Unrecognized("Cannot split " + name);
    }

    // split for names looking like international but with unrecognized "garbage" (extra) at the end
    inline void split_with_extra(std::string name, std::string& virus_type, std::string& host, std::string& location, std::string& isolation, std::string& year, std::string& passage,
                                 std::string& extra)
    {
        try {
            split(name, virus_type, host, location, isolation, year, passage);
        }
        catch (Unrecognized&) {
            std::smatch m;
            if (std::regex_search(name, m, _internal::international_name_with_extra)) {
                virus_type = m[1].str();
                host = m[2].str();
                location = m[3].str();
                isolation = m[4].str();
                year = _internal::make_year(m);
                extra = m[7].str();
            }
            else
                throw Unrecognized("Cannot split " + name);
        }
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

} // namespace virus_name

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
