#include <cstdlib>
#include <chrono>
#include <thread>
#include <unistd.h>

#include "quicklook.hh"

// ----------------------------------------------------------------------

#ifdef __APPLE__

void acmacs::quicklook(std::string aFilename, size_t aDelayInSeconds)
{
    const char * const argv[] = {"qlmanage", "-p", aFilename.c_str(), nullptr};
    run_and_detach(argv);
    if (aDelayInSeconds) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s * aDelayInSeconds);
    }

} // acmacs::quicklook

#else

void acmacs::quicklook(std::string /*aFilename*/, size_t /*aDelayInSeconds*/)
{
}

#endif

// ----------------------------------------------------------------------

void acmacs::run_and_detach(const char * const argv[])
{
    if (!fork()) {
        close(0);
        close(1);
        close(2);
        setsid();
        execvp(argv[0], const_cast<char *const *>(argv));
        perror(argv[0]);
        std::exit(-1);
    }

} // acmacs::run_and_detach

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
