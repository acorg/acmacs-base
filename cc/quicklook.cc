#include <cstdlib>
#include <sstream>
#include <chrono>
#include <thread>

#include "quicklook.hh"

// ----------------------------------------------------------------------

#ifdef __APPLE__

void acmacs::quicklook(std::string aFilename, size_t aDelayInSeconds)
{
    std::ostringstream cmd;
    cmd << "qlmanage -p '" << aFilename << "' >/dev/null 2>&1 &";
    system(cmd.str().c_str());
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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
