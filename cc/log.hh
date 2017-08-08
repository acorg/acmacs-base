#pragma once

#include "acmacs-base/target.hh"

// ----------------------------------------------------------------------

#ifdef ACMACS_TARGET_BROWSER
#include "asm.hh"
#endif

// ----------------------------------------------------------------------

#ifdef ACMACS_TARGET_OS
#include <iostream>

namespace log_internal
{
    template<typename ...Args> void /*std::ostream&*/ write(std::ostream& out, Args&& ... args)
    {
        (void)std::initializer_list<int>{(out << std::forward<Args>(args), void(), 0)... };
        out << std::endl;
          // return out;
    }
}

template <typename ... Args> inline void log(Args&& ... rest) { log_internal::write(std::cout, "INFO: ", rest ...); }
template <typename ... Args> inline void log_error(Args ... rest) { log_internal::write(std::cerr, "ERROR: ", rest ...); }
template <typename ... Args> inline void log_warning(Args ... rest) { log_internal::write(std::cerr, "WARNING: ", rest ...); }

#endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
