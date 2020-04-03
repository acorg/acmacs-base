#pragma once

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
// #elif __has_include(<experimental/filesystem>)
// #include <experimental/filesystem>
// namespace fs = std::experimental::filesystem;
#else
#error No std::filesystem
// #pragma GCC diagnostic push
// #include "acmacs-base/boost-diagnostics.hh"
// #include "boost/filesystem.hpp"
// #pragma GCC diagnostic pop
// namespace fs = boost::filesystem;
#endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
