#pragma once

#include <optional>

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    void quicklook(std::string_view aFilename, size_t aDelayInSeconds = 0, std::optional<size_t> aRemoveAfterDelay = std::nullopt);
    void open(std::string_view aFilename, size_t aDelayBeforeInSeconds = 0, size_t aDelayAfterInSeconds = 0, std::optional<size_t> aRemoveAfterDelay = std::nullopt);
    void preview(std::string_view aFilename, std::string_view position, size_t aDelayBeforeInSeconds = 0, size_t aDelayAfterInSeconds = 0);

    void run_and_detach_with_delay(const std::vector<std::string>& argv, size_t aDelayBeforeInSeconds = 0);

    template <typename ... Args> inline void run_and_detach(Args&& ... args)
    {
        const std::vector<std::string> va{fmt::format("{}", std::forward<Args>(args)) ...};
        run_and_detach_with_delay(va);
    }

    inline void open_or_quicklook(bool do_open, bool do_quicklook, std::string_view aFilename, size_t aDelayBeforeInSeconds = 0, size_t aDelayAfterInSeconds = 2, std::optional<size_t> aRemoveAfterDelay = std::nullopt)
    {
        if (do_open)
            open(aFilename, aDelayBeforeInSeconds, aDelayAfterInSeconds, aRemoveAfterDelay);
        else if (do_quicklook)
            quicklook(aFilename, aDelayBeforeInSeconds, aRemoveAfterDelay);
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
