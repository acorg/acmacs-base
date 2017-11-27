#pragma once

#include <string>

// ----------------------------------------------------------------------

namespace acmacs
{
    void quicklook(std::string aFilename, size_t aDelayInSeconds = 0);
    void open(std::string aFilename, size_t aDelayBeforeInSeconds = 0, size_t aDelayAfterInSeconds = 0);
    void run_and_detach(const char * const argv[], size_t aDelayBeforeInSeconds);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
