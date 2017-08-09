#pragma once

#include "acmacs-base/log.hh"

// ----------------------------------------------------------------------
// Browser
// ----------------------------------------------------------------------

#ifdef ACMACS_TARGET_BROWSER
#undef ACMACSD_FILESYSTEM
#define NO_EXCEPTIONS
#define DYNAMIC_CAST(Target,Source) ((Target)(Source))
#define union struct

#define typeid typeid_simulation_for_cheerp
struct s_typeid_simulation_for_cheerp
{
    const char* name() const { return "No-RTTI-in-chirp"; }
};
template <typename T> s_typeid_simulation_for_cheerp typeid_simulation_for_cheerp(const T&) { return s_typeid_simulation_for_cheerp{}; }

#define THROW(exc, cheerp_result) log_warning("THROW: ", (exc).what()); return (cheerp_result)
#define THROW_OR_VOID(exc) return
#define THROW_OR_CERR(exc) log_error("ERROR: ", (exc).what()); return
//#define THROW_OR_CERR_WITHOUT_RETURN(exc) log_error("ERROR: ", (exc).what())
#endif

// ----------------------------------------------------------------------
// OS
// ----------------------------------------------------------------------

#ifdef ACMACS_TARGET_OS
#define ACMACSD_FILESYSTEM
#define DYNAMIC_CAST(Target,Source) dynamic_cast<Target>(Source)
#define THROW(exc, cheerp_result) throw (exc)
#define THROW_OR_VOID(exc) throw (exc)
#define THROW_OR_CERR(exc) throw (exc)
//#define THROW_OR_CERR_WITHOUT_RETURN(exc) throw (exc)
#endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
