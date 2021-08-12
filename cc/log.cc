#include <map>

#include "acmacs-base/log.hh"
#include "acmacs-base/range-v3.hh"
#include "acmacs-base/string-split.hh"

size_t acmacs::log::v1::detail::indent{0};
bool acmacs::log::v1::detail::print_debug_messages{true}; // to disable by acmacs.r

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

namespace acmacs::log::inline v1
{
    detail::enabled_t detail::enabled{};

    const log_key_t all{"all"};
    const log_key_t timer{"timer"};
    const log_key_t settings{"settings"};
    const log_key_t vaccines{"vaccines"};

} // namespace acmacs::log::v1

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

void acmacs::log::v1::enable(std::string_view names)
{
    using namespace std::string_view_literals;

    for (const auto& name : string::split(names, ","sv, string::Split::RemoveEmpty)) {
        if (name == all) {
            if (detail::enabled.empty() || detail::enabled.front() != name)
                detail::enabled.insert(detail::enabled.begin(), log_key_t{name});
        }
        else if (std::find(std::begin(detail::enabled), std::end(detail::enabled), name) == std::end(detail::enabled))
            detail::enabled.push_back(log_key_t{name});
    }

} // acmacs::log::v1::enable

// ----------------------------------------------------------------------

void acmacs::log::v1::enable(const std::vector<std::string_view>& names)
{
    for (const auto& name : names)
        enable(name);

} // acmacs::log::v1::enable

// ----------------------------------------------------------------------

std::string acmacs::log::v1::report_enabled()
{
    fmt::memory_buffer out;
    fmt::format_to_mb(out, "log messages enabled for ({}):", detail::enabled.size());
    for (const auto& key : detail::enabled)
        fmt::format_to_mb(out, " \"{}\"", static_cast<std::string_view>(key));
    return fmt::to_string(out);

} // acmacs::log::v1::report_enabled

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
