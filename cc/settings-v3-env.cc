#include "acmacs-base/settings-v3-env.hh"
#include "acmacs-base/settings-v3.hh"
#include "acmacs-base/enumerate.hh"

// ----------------------------------------------------------------------

const rjson::v3::value& acmacs::settings::v3::detail::Environment::get(std::string_view key, toplevel_only a_toplevel_only) const
{
    const auto final_key = substitute_to_string(key);
    AD_LOG(acmacs::log::settings, "env get \"{}\" (\"{}\")", final_key, key);
    switch (a_toplevel_only) {
        case toplevel_only::no:
            for (auto it = env_data_.rbegin(); it != env_data_.rend(); ++it) {
                if (auto found = it->find(final_key); found != it->end())
                    return substitute(found->second);
            }
            break;
        case toplevel_only::yes:
            if (auto found = env_data_.back().find(final_key); found != env_data_.back().end())
                return substitute(found->second);
            break;
    }
    return rjson::v3::const_null;

} // acmacs::settings::v3::detail::Environment::get

// ----------------------------------------------------------------------

const rjson::v3::value& acmacs::settings::v3::detail::Environment::substitute(std::string_view text, const rjson::v3::value& source) const
{
    if (text.size() > 1 && text[0] == '$')
        return get(text.substr(1));

#include "acmacs-base/global-constructors-push.hh"
    static const std::regex re{"\\{([^\\{}]+)\\}"};
#include "acmacs-base/diagnostics-pop.hh"

    std::string result{text};
    std::smatch m1;
    bool result_modified{false};
    for (ssize_t start{0}; static_cast<size_t>(start) < result.size() && std::regex_search(std::next(std::cbegin(result), start), std::cend(result), m1, re);) {
        const auto replacement_start = start + m1.position();
        if (const auto& found1 = get(m1.str(1)); !found1.is_null()) {
            result.erase(static_cast<size_t>(replacement_start), static_cast<size_t>(m1.length()));
            result.insert(static_cast<size_t>(replacement_start), found1.as_string());
            result_modified = true;
        }
        else // no replacement found, leave as is
            start = replacement_start + m1.length();
    }
    if (result_modified) {
#include "acmacs-base/global-constructors-push.hh"
        static thread_local rjson::v3::value substituted;
#include "acmacs-base/diagnostics-pop.hh"
        substituted = rjson::v3::detail::string(rjson::v3::detail::string::with_content, std::move(result));
        AD_LOG(acmacs::log::settings, "env substituted \"{}\" -> {}", text, substituted);
        return substituted;
    }
    else {
        AD_LOG(acmacs::log::settings, "env substituted nothing \"{}\" -> {}", text, source);
        return source;
    }

} // acmacs::settings::v3::detail::Environment::substitute

// ----------------------------------------------------------------------

const rjson::v3::value& acmacs::settings::v3::detail::Environment::substitute(const rjson::v3::value& source) const
{
    return source.visit([this, &source]<typename T>(const T& val) -> const rjson::v3::value& {
        if constexpr (std::is_same_v<T, rjson::v3::detail::string>)
            return substitute(val.template to<std::string_view>(), source);
        else
            return source;
    });

} // acmacs::settings::v3::detail::Environment::substitute

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
