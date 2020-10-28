#include <cstdlib>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <vector>

#include "acmacs-base/filesystem.hh"
#include "acmacs-base/quicklook.hh"
#include "acmacs-base/log.hh"

// ----------------------------------------------------------------------

#ifdef __APPLE__

inline bool return_to_emacs()
{
    if (const auto* xpc = std::getenv("XPC_SERVICE_NAME"), *inside = std::getenv("INSIDE_EMACS"); (xpc && std::string_view{xpc}.find("Emacs") != std::string_view::npos) || inside)
        return true;
    else
        return false;
}

void acmacs::quicklook(std::string_view aFilename, size_t aDelayInSeconds)
{
    run_and_detach({"/usr/bin/qlmanage", "-p", aFilename.data()}, 0);
    if (aDelayInSeconds) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s * aDelayInSeconds);
    }

} // acmacs::quicklook

// ----------------------------------------------------------------------

void acmacs::open(std::string_view aFilename, size_t aDelayBeforeInSeconds, size_t aDelayAfterInSeconds)
{
    using namespace std::chrono_literals;

    if (aDelayBeforeInSeconds)
        run_and_detach({"/usr/bin/open", aFilename.data()}, aDelayBeforeInSeconds); // , "-g"
    else
        std::system(fmt::format("/usr/bin/open '{}'", aFilename).data());

    if (return_to_emacs())
        run_and_detach({"/usr/bin/open", "-a", "/Applications/Emacs.app"}, aDelayBeforeInSeconds + 1);

    if (aDelayAfterInSeconds)
        std::this_thread::sleep_for(1s * aDelayAfterInSeconds);

} // acmacs::quicklook

// ----------------------------------------------------------------------

void acmacs::preview(std::string_view aFilename, std::string_view position, size_t aDelayBeforeInSeconds, size_t aDelayAfterInSeconds)
{
    using namespace std::chrono_literals;

    if (const fs::path preview_path{fmt::format("{}/bin/preview", std::getenv("HOME"))}; fs::exists(preview_path)) {

        if (aDelayBeforeInSeconds)
            run_and_detach({preview_path.c_str(), "-p", position.data(), aFilename.data()}, aDelayBeforeInSeconds); // , "-g"
        else
            std::system(fmt::format("{} -p {} '{}'", preview_path, position, aFilename).data());

        if (return_to_emacs())
            run_and_detach({"/usr/bin/open", "-a", "/Applications/Emacs.app"}, aDelayBeforeInSeconds + 1);

        if (aDelayAfterInSeconds)
            std::this_thread::sleep_for(1s * aDelayAfterInSeconds);
    }
    else
        open(aFilename, aDelayBeforeInSeconds, aDelayAfterInSeconds);

} // acmacs::preview

// ======================================================================

#else

void acmacs::quicklook(std::string_view /*aFilename*/, size_t /*aDelayInSeconds*/)
{
}

void acmacs::open(std::string_view /*aFilename*/, size_t /*aDelayBeforeInSeconds*/, size_t /*aDelayAfterInSeconds*/)
{
}

void acmacs::preview(std::string_view /*aFilename*/, std::string_view /*position*/, size_t /*aDelayBeforeInSeconds*/, size_t /*aDelayAfterInSeconds*/)
{
}

#endif

// ======================================================================

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
