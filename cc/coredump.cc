#include <sys/resource.h>
#include <cstring>
#include <cerrno>

#include "acmacs-base/coredump.hh"
#include "acmacs-base/log.hh"

// ----------------------------------------------------------------------

#ifdef __APPLE__

void acmacs::enable_coredump()
{
    // https://developer.apple.com/forums/thread/127503?answerId=401103022#401103022
    // sudo chmod 1777 /cores

    struct rlimit limit;
    if (!getrlimit(RLIMIT_CORE, &limit)) {
        limit.rlim_cur = limit.rlim_max;
        if (setrlimit(RLIMIT_CORE, &limit))
            AD_WARNING("setrlimit failed (coredumps not enabled): {}", std::strerror(errno));
    }
    else
        AD_WARNING("getrlimit failed (coredumps not enabled): {}", std::strerror(errno));

} // acmacs::enable_coredump

#else

void acmacs::enable_coredump()
{
}

#endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
