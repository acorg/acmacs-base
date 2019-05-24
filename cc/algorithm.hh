#pragma once

#include <algorithm>

// ----------------------------------------------------------------------

namespace acmacs
{
    template <typename Input, typename Output, typename Pred, typename Oper> inline Output transform_if(Input first, Input last, Output target, Pred predicate, Oper operation)
    {
        for (; first != last; ++first) {
            if (predicate(*first))
                *target++ = operation(*first);
        }
        return target;
    }

} // namespace acmacs


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
