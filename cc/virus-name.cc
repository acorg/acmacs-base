#include <iostream>
#include <cctype>
#include <regex>

#include "acmacs-base/virus-name.hh"
#include "acmacs-base/string.hh"

// ----------------------------------------------------------------------

namespace virus_name
{
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

    void split_and_strip(std::string name, std::string& virus_type, std::string& host, std::string& location, std::string& isolation, std::string& year, std::string& extra)
    {
        std::smatch m;
        if (std::regex_match(name, m, _internal::international_name_with_extra)) {
            virus_type = string::strip(m[1].str());
            host = string::strip(m[2].str());
            location = string::strip(m[3].str());
            isolation = string::strip(m[4].str());
            year = _internal::make_year(m);
            extra = string::strip(m[7].str());
        }
        else
            throw Unrecognized("Cannot split " + name);
    }

    // split for names looking like international but with unrecognized "garbage" (extra) at the end
    void split_with_extra(std::string name, std::string& virus_type, std::string& host, std::string& location, std::string& isolation, std::string& year, std::string& passage, std::string& extra)
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

    Name::Name(std::string source)
    {
        try {
            split_and_strip(source, virus_type, host, location, isolation, year, extra);
        }
        catch (Unrecognized&) {
            bool re_throw = true;
            if (const auto slashes = std::count(std::begin(source), std::end(source), '/'); slashes == 2) {
                if (const auto num_start = std::find_if(std::begin(source), std::end(source), [](char cc) { return std::isdigit(cc); });
                    num_start != std::begin(source) && num_start != std::end(source) && *(num_start - 1) != '/') {
                    // A/PTO MONTT75856/2015
                    source.insert(num_start, '/');
                    split_and_strip(source, virus_type, host, location, isolation, year, extra);
                    re_throw = false;
                }
            }
            if (re_throw)
                throw;
        }
        if (const auto first_not_zero = isolation.find_first_not_of('0'); first_not_zero != std::string::npos)
            isolation.erase(0, first_not_zero);
    }

    void Name::fix_extra()
    {
        if (std::string_view(extra.data(), std::min(extra.size(), 8UL)) == "NYMC BX-")
            extra = "NYMC-" + extra.substr(8);
        if (!extra.empty())
            std::cerr << "WARNING: name contains extra: \"" << join() << "\"\n";
    }

    std::string Name::join() const
    {
        std::string result;
        if (host.empty())
            result = string::join("/", {virus_type, location, isolation, year});
        else
            result = string::join("/", {virus_type, host, location, isolation, year});
        if (!extra.empty()) {
            result.append(1, ' ');
            result.append(extra);
        }
        return result;
    }

    // std::string normalize(std::string name)
    // {
    //     std::string result;
    //     std::string virus_type, host, location, isolation, year, passage;
    //     try {
    //         split(name, virus_type, host, location, isolation, year, passage);

    //         virus_type = string::strip(virus_type);
    //         host = string::strip(host);
    //         location = string::strip(location);
    //         isolation = string::strip(isolation);
    //         auto first_not_zero = isolation.find_first_not_of('0');
    //         if (first_not_zero != std::string::npos)
    //             isolation.erase(0, first_not_zero);
    //         year = string::strip(year);
    //         passage = string::strip(passage);
    //         if (host.empty())
    //             result = string::join("/", {virus_type, location, isolation, year});
    //         else
    //             result = string::join("/", {virus_type, host, location, isolation, year});
    //         if (!passage.empty()) {
    //             std::cerr << "WARNING: name contains extra: \"" << name << '"' << std::endl;
    //             result.append(1, ' ');
    //             result.append(passage);
    //         }
    //     }
    //     catch (Unrecognized&) {
    //         std::cerr << "WARNING: Virus name unrecognized: \"" << name << '"' << std::endl;
    //         result = name;
    //     }
    //     return result;

    // } // normalize

} // namespace virus_name

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
