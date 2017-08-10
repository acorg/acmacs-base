#pragma once

// ----------------------------------------------------------------------

#ifdef __CHEERP_CLIENT__
#define ACMACS_TARGET_BROWSER 1
#else
#define ACMACS_TARGET_OS 1
#endif

// ----------------------------------------------------------------------

#ifdef ACMACS_TARGET_BROWSER
#undef ACMACSD_FILESYSTEM
#endif

#ifdef ACMACS_TARGET_OS
#define ACMACSD_FILESYSTEM
#endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
