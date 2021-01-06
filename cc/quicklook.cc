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

template <typename T> static inline std::string command_with_path(T&& cmd)
{
    return fmt::format("{}/bin/{}", std::getenv("ACMACSD_ROOT"), std::forward<T>(cmd));
}

void acmacs::quicklook(std::string_view aFilename, size_t aDelayInSeconds, std::optional<size_t> aRemoveAfterDelay)
{
    if (aRemoveAfterDelay.has_value())
        run_and_detach(command_with_path("open-with-delay"), "-q", "-a", aDelayInSeconds, "-r", *aRemoveAfterDelay, aFilename);
    else
        run_and_detach(command_with_path("open-with-delay"), "-q", "-a", aDelayInSeconds, aFilename);

} // acmacs::quicklook

// ----------------------------------------------------------------------

void acmacs::open(std::string_view aFilename, size_t aDelayBeforeInSeconds, size_t aDelayAfterInSeconds, std::optional<size_t> aRemoveAfterDelay)
{
    if (aRemoveAfterDelay.has_value())
        run_and_detach(command_with_path("open-with-delay"), "-e", "-b", aDelayBeforeInSeconds, "-a", aDelayAfterInSeconds, "-r", *aRemoveAfterDelay, aFilename);
    else
        run_and_detach(command_with_path("open-with-delay"), "-e", "-b", aDelayBeforeInSeconds, "-a", aDelayAfterInSeconds, aFilename);

} // acmacs::quicklook

// ----------------------------------------------------------------------

void acmacs::preview(std::string_view aFilename, std::string_view position, size_t aDelayBeforeInSeconds, size_t aDelayAfterInSeconds)
{
    run_and_detach(command_with_path("preview"), "-p", position, "-e", "-b", aDelayBeforeInSeconds, "-a", aDelayAfterInSeconds, aFilename);

} // acmacs::preview

// ----------------------------------------------------------------------

// inline bool return_to_emacs()
// {
//     if (const auto* xpc = std::getenv("XPC_SERVICE_NAME"), *inside = std::getenv("INSIDE_EMACS"); (xpc && std::string_view{xpc}.find("Emacs") != std::string_view::npos) || inside)
//         return true;
//     else
//         return false;
// }

// // ----------------------------------------------------------------------

// void acmacs::quicklook(std::string_view aFilename, size_t aDelayInSeconds, std::optional<size_t> aRemoveAfterDelay)
// {
//     run_and_detach_with_delay({"/usr/bin/qlmanage", "-p", aFilename.data()}, 0);
//     if (aDelayInSeconds) {
//         using namespace std::chrono_literals;
//         std::this_thread::sleep_for(1s * aDelayInSeconds);
//     }

// } // acmacs::quicklook

// // ----------------------------------------------------------------------

// void acmacs::open(std::string_view aFilename, size_t aDelayBeforeInSeconds, size_t aDelayAfterInSeconds, std::optional<size_t> aRemoveAfterDelay)
// {
//     using namespace std::chrono_literals;

//     if (aDelayBeforeInSeconds)
//         run_and_detach_with_delay({"/usr/bin/open", aFilename.data()}, aDelayBeforeInSeconds); // , "-g"
//     else
//         std::system(fmt::format("/usr/bin/open '{}'", aFilename).data());

//     if (return_to_emacs())
//         run_and_detach_with_delay({"/usr/bin/open", "-a", "/Applications/Emacs.app"}, aDelayBeforeInSeconds + 1);

//     if (aDelayAfterInSeconds)
//         std::this_thread::sleep_for(1s * aDelayAfterInSeconds);

// } // acmacs::quicklook

// ----------------------------------------------------------------------

// void acmacs::preview(std::string_view aFilename, std::string_view position, size_t aDelayBeforeInSeconds, size_t aDelayAfterInSeconds)
// {
//     using namespace std::chrono_literals;

//     if (const fs::path preview_path{fmt::format("{}/bin/preview", std::getenv("HOME"))}; fs::exists(preview_path)) {

//         if (aDelayBeforeInSeconds)
//             run_and_detach_with_delay({preview_path.c_str(), "-p", position.data(), aFilename.data()}, aDelayBeforeInSeconds); // , "-g"
//         else
//             std::system(fmt::format("{} -p {} '{}'", preview_path, position, aFilename).data());

//         if (return_to_emacs())
//             run_and_detach_with_delay({"/usr/bin/open", "-a", "/Applications/Emacs.app"}, aDelayBeforeInSeconds + 1);

//         if (aDelayAfterInSeconds)
//             std::this_thread::sleep_for(1s * aDelayAfterInSeconds);
//     }
//     else
//         open(aFilename, aDelayBeforeInSeconds, aDelayAfterInSeconds);

// } // acmacs::preview

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

void acmacs::run_and_detach_with_delay(const std::vector<std::string>& argv, size_t aDelayBeforeInSeconds)
{
    if (!fork()) {
        close(0);
        // close(1);
        // close(2);
        setsid();
        if (aDelayBeforeInSeconds) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s * aDelayBeforeInSeconds);
        }
        std::vector<const char *> argv_v(argv.size() + 1, nullptr);
        std::transform(std::begin(argv), std::end(argv), std::begin(argv_v), [](const auto& aa) { return aa.data(); });
        execvp(argv_v[0], const_cast<char *const *>(argv_v.data()));
        perror(argv_v[0]);
        std::exit(-1);
    }

} // acmacs::run_and_detach_with_delay

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
