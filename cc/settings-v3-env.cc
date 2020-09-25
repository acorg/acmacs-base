#include "acmacs-base/settings-v3-env.hh"
#include "acmacs-base/settings-v3.hh"
#include "acmacs-base/enumerate.hh"

// ----------------------------------------------------------------------

const rjson::v3::value& acmacs::settings::v3::detail::Environment::get(std::string_view key, toplevel_only a_toplevel_only) const
{
    switch (a_toplevel_only) {
        case toplevel_only::no:
            for (auto it = env_data_.rbegin(); it != env_data_.rend(); ++it) {
                if (auto found = it->find(key); found != it->end())
                    return found->second;
            }
            break;
        case toplevel_only::yes:
            if (auto found = env_data_.back().find(key); found != env_data_.back().end())
                return found->second;
            break;
    }
    return rjson::v3::const_null;

} // acmacs::settings::v3::detail::Environment::get

// ----------------------------------------------------------------------

std::string acmacs::settings::v3::detail::Environment::substitute_to_string(std::string_view source) const
{
    return std::string { source };

} // acmacs::settings::v3::detail::Environment::substitute_to_string

// ----------------------------------------------------------------------

std::string acmacs::settings::v3::detail::Environment::format(std::string_view indent) const
{
    fmt::memory_buffer out;
    for (auto [level, entries] : acmacs::enumerate(env_data_)) {
        for (const auto& entry : entries)
            fmt::format_to(out, "{}\"{}\": {} <{}>\n", indent, entry.first, entry.second, level);
    }
    return fmt::to_string(out);

} // acmacs::settings::v3::detail::Environment::format

// ----------------------------------------------------------------------

void acmacs::settings::v3::detail::LoadedDataFiles::load(std::string_view filename)
{
    file_data_.insert(file_data_.begin(), rjson::v3::parse_file(filename));
    filenames_.insert(filenames_.begin(), std::string{filename});

} // acmacs::settings::v3::detail::LoadedDataFiles::load

// ----------------------------------------------------------------------

void acmacs::settings::v3::detail::LoadedDataFiles::reload(Data& settings)
{
    using namespace std::string_view_literals;
    for (auto index{filenames_.size()}; index > 0; --index) {
        if (!filenames_[index - 1].empty()) {
            AD_LOG(acmacs::log::settings, "re-loading {}", filenames_[index - 1]);
            file_data_[index - 1] = rjson::v3::parse_file(filenames_[index - 1]);
        }
        if (const auto& val = file_data_[index - 1]["init"sv]; !val.is_null())
            settings.apply(val);
    }

} // acmacs::settings::v3::detail::LoadedDataFiles::reload

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
