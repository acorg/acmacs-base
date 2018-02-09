#include <cstdlib>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <iostream>

#include "quicklook.hh"

// ----------------------------------------------------------------------

#ifdef __APPLE__

void acmacs::quicklook(std::string aFilename, size_t aDelayInSeconds)
{
    const char * const argv[] = {"/usr/bin/qlmanage", "-p", aFilename.c_str(), nullptr};
    run_and_detach(argv, 0);
    if (aDelayInSeconds) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s * aDelayInSeconds);
    }

} // acmacs::quicklook

void acmacs::open(std::string aFilename, size_t aDelayBeforeInSeconds, size_t aDelayAfterInSeconds)
{
    const char * const argv[] = {"/usr/bin/open", aFilename.c_str(), nullptr};
    run_and_detach(argv, aDelayBeforeInSeconds);
    if (aDelayAfterInSeconds) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s * aDelayAfterInSeconds);
    }

} // acmacs::quicklook

#else

void acmacs::quicklook(std::string /*aFilename*/, size_t /*aDelayInSeconds*/)
{
}

void acmacs::open(std::string /*aFilename*/, size_t /*aDelayBeforeInSeconds*/, size_t /*aDelayAfterInSeconds*/)
{
}

#endif

// ----------------------------------------------------------------------

void acmacs::run_and_detach(const char * const argv[], size_t aDelayBeforeInSeconds)
{
    if (!fork()) {
        close(0);
        close(1);
        close(2);
        setsid();
        if (aDelayBeforeInSeconds) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s * aDelayBeforeInSeconds);
        }
        execvp(argv[0], const_cast<char *const *>(argv));
        perror(argv[0]);
        std::exit(-1);
    }

} // acmacs::run_and_detach

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
