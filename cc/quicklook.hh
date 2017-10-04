#pragma once

#include <string>

// ----------------------------------------------------------------------

namespace acmacs
{
    void quicklook(std::string aFilename, size_t aDelayInSeconds = 0);
    void run_and_detach(const char * const argv[]);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
