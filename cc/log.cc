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

acmacs::log::v1::detail::enabled_t acmacs::log::v1::detail::enabled{};

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

void acmacs::log::v1::enable(std::string_view names)
{
    using namespace std::string_view_literals;

    for (const auto& name : string::split(names, ","sv, string::Split::RemoveEmpty)) {
        if (name == all) {
            if (detail::enabled.empty() || detail::enabled.front() != name)
                detail::enabled.insert(detail::enabled.begin(), name);
        }
        else if (std::find(std::begin(detail::enabled), std::end(detail::enabled), name) == std::end(detail::enabled))
            detail::enabled.push_back(name);
    }

} // acmacs::log::v1::enable

// ----------------------------------------------------------------------

void acmacs::log::v1::enable(const std::vector<std::string_view>& names)
{
    for (const auto& name : names)
        enable(name);

} // acmacs::log::v1::enable

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
