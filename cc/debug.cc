#include <map>

#include "acmacs-base/debug.hh"
#include "acmacs-base/range-v3.hh"
#include "acmacs-base/string-split.hh"

acmacs::log::v1::detail::section_t acmacs::log::v1::detail::enabled{0};

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

static std::map<std::string_view, acmacs::log::v1::detail::section_t> enablers;

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

namespace acmacs::log::inline v1
{
    constexpr const acmacs::log::v1::detail::section_t log_all{0xFFFFFFFF};

    template <typename... Sec> constexpr void enable(detail::section_t en, Sec... rest)
    {
        detail::enabled |= en;
        if constexpr (sizeof...(rest) > 0)
            enable(rest...);
    }

} // namespace acmacs::log::inlinev1

// ----------------------------------------------------------------------

void acmacs::log::v1::register_enabler_acmacs_base()
{
    using namespace std::string_view_literals;
    acmacs::log::detail::register_enabler("all"sv, log_all);
    acmacs::log::register_enabler("timer"sv, acmacs::log::timer);
    acmacs::log::register_enabler("settings"sv, acmacs::log::settings);

} // acmacs::log::v1::register_enabler_acmacs_base

// ----------------------------------------------------------------------

std::string acmacs::log::v1::detail::section_names(section_t section)
{
    return enablers
            | ranges::views::filter([section](const auto& en) { return (en.second & section) != 0; })
            | ranges::views::filter([](const auto& en) { return en.second != log_all; })
            | ranges::views::transform([](const auto& en) { return en.first; })
            | ranges::views::join(',')
            | ranges::to<std::string>
            ;

} // acmacs::log::v1::detail::section_names

// ----------------------------------------------------------------------

void acmacs::log::v1::detail::register_enabler(std::string_view name, section_t value)
{
    if (const auto found = enablers.find(name); found != enablers.end())
        throw std::runtime_error{fmt::format("log enabler \"{}\" already registered", name)};
    enablers.emplace(name, value);

} // acmacs::log::v1::detail::register_enabler

// ----------------------------------------------------------------------

void acmacs::log::v1::enable(std::string_view names)
{
    using namespace std::string_view_literals;

    for (const auto& name : string::split(names, ","sv, string::Split::RemoveEmpty)) {
        if (const auto found = enablers.find(name); found == enablers.end()) {
            fmt::print(stderr, "WARNING: available log enablers:\n");
            for (const auto& [enabler_name,  _] : enablers)
                fmt::print(stderr, "    {}\n", enabler_name);
            throw std::runtime_error{fmt::format("log enabler \"{}\" not registered", name)};
        }
        else
            enable(found->second);
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