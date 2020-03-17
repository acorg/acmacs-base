#include <map>

#include "acmacs-base/debug.hh"
#include "acmacs-base/string-split.hh"

acmacs::log::v1::section_t acmacs::log::v1::enabled{0};

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

static std::map<std::string_view, acmacs::log::v1::section_t> enablers;

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

void acmacs::log::v1::register_enabler(std::string_view name, section_t value)
{
    if (const auto found = enablers.find(name); found != enablers.end())
        throw std::runtime_error{fmt::format("log enabler \"{}\" already registered", name)};
    enablers.emplace(name, value);

} // acmacs::log::v1::register_enabler

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
