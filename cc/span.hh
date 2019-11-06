#pragma once

#if __has_include(<span>)

#include <span>

#else

#include "acmacs-base/span.hpp"
namespace std
{
    using namespace tcb;
}

#endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
