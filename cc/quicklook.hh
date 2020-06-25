#pragma once

#include <string_view>

// ----------------------------------------------------------------------

namespace acmacs
{
    void quicklook(std::string_view aFilename, size_t aDelayInSeconds = 0);
    void open(std::string_view aFilename, size_t aDelayBeforeInSeconds = 0, size_t aDelayAfterInSeconds = 0);
    void preview(std::string_view aFilename, std::string_view position, size_t aDelayBeforeInSeconds = 0, size_t aDelayAfterInSeconds = 0);
    void run_and_detach(std::initializer_list<const char *> argv, size_t aDelayBeforeInSeconds);

    inline void open_or_quicklook(bool do_open, bool do_quicklook, std::string_view aFilename, size_t aDelayBeforeInSeconds = 0, size_t aDelayAfterInSeconds = 2)
    {
        if (do_open)
            open(aFilename, aDelayBeforeInSeconds, aDelayAfterInSeconds);
        else if (do_quicklook)
            quicklook(aFilename, aDelayBeforeInSeconds);
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
