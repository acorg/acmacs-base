#pragma once

#include <filesystem>
namespace fs = std::filesystem;

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

template <> struct fmt::formatter<std::filesystem::path> : fmt::formatter<const char*> {
    template <typename FormatCtx> auto format(const std::filesystem::path& path, FormatCtx& ctx) { return fmt::formatter<const char*>::format(path.c_str(), ctx); }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
