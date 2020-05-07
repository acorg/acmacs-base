#include <cstdlib>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <vector>

#include "acmacs-base/quicklook.hh"
#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

#ifdef __APPLE__

void acmacs::quicklook(std::string_view aFilename, size_t aDelayInSeconds)
{
    run_and_detach({"/usr/bin/qlmanage", "-p", aFilename.data()}, 0);
    if (aDelayInSeconds) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s * aDelayInSeconds);
    }

} // acmacs::quicklook

void acmacs::open(std::string_view aFilename, size_t aDelayBeforeInSeconds, size_t aDelayAfterInSeconds)
{
    if (aDelayBeforeInSeconds)
        run_and_detach({"/usr/bin/open", aFilename.data()}, aDelayBeforeInSeconds); // , "-g"
    else
        std::system(fmt::format("/usr/bin/open '{}'", aFilename).data());

    if (const auto* xpc = std::getenv("XPC_SERVICE_NAME"), *inside = std::getenv("INSIDE_EMACS"); (xpc && std::string_view{xpc}.find("Emacs") != std::string_view::npos) || inside)
        // run_and_detach({"/usr/bin/open", "-a", "/Applications/Emacs.app"}, aDelayBeforeInSeconds + 2);
        std::system(fmt::format("( sleep {}; /usr/bin/open -a /Applications/Emacs.app ) &", aDelayBeforeInSeconds + 1).data());
    if (aDelayAfterInSeconds) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s * aDelayAfterInSeconds);
    }

} // acmacs::quicklook

#else

void acmacs::quicklook(std::string_view /*aFilename*/, size_t /*aDelayInSeconds*/)
{
}

void acmacs::open(std::string_view /*aFilename*/, size_t /*aDelayBeforeInSeconds*/, size_t /*aDelayAfterInSeconds*/)
{
}

#endif

// ----------------------------------------------------------------------

void acmacs::run_and_detach(std::initializer_list<const char *> argv, size_t aDelayBeforeInSeconds)
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
        std::vector<const char *> argv_v{argv};
        argv_v.push_back(nullptr);
        execvp(argv_v[0], const_cast<char *const *>(argv_v.data()));
        perror(argv_v[0]);
        std::exit(-1);
    }

} // acmacs::run_and_detach

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
