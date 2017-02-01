#pragma once

#include <regex>

// ----------------------------------------------------------------------

namespace passage
{
    inline bool is_egg(std::string aPassage)
    {
#include "acmacs-base/global-constructors-push.hh"
        static std::regex egg_passage{
            R"#(E(\?|[0-9][0-9]?))#"  // passage
                    R"#(( (ISOLATE|CLONE) [0-9\-]+)*)#"         // NIMR isolate and/or clone, NIMR H1pdm has CLONE 38-32
                    R"#(( *\+[1-9])?)#"         // NIID has +1 at the end of passage
                    R"#(( \([12][0129][0-9][0-9]-[01][0-9]-[0-3][0-9]\))?$)#" // passage date
                    };
#include "acmacs-base/diagnostics-pop.hh"
        return std::regex_search(aPassage, egg_passage);

    } // is_egg

// ----------------------------------------------------------------------

    inline std::string without_date(std::string aPassage)
    {
        if (aPassage.size() > 13 && aPassage[aPassage.size() - 1] == ')' && aPassage[aPassage.size() - 12] == '(' && aPassage[aPassage.size() - 13] == ' ' && aPassage[aPassage.size() - 4] == '-' && aPassage[aPassage.size() - 7] == '-')
            return std::string(aPassage, 0, aPassage.size() - 13);
        else
            return aPassage;
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
