#include "acmacs-base/settings-v3-env.hh"
#include "acmacs-base/settings-v3.hh"
#include "acmacs-base/enumerate.hh"

// ----------------------------------------------------------------------

const rjson::v3::value& acmacs::settings::v3::detail::Environment::get(std::string_view key, toplevel_only a_toplevel_only) const
{
    const auto final_key = substitute(key);
    AD_LOG(acmacs::log::settings, "env get \"{}\" (\"{}\") toplevel_only:{}", final_key, key, a_toplevel_only == toplevel_only::yes ? "yes" : "no");
    AD_LOG_INDENT;
    switch (a_toplevel_only) {
        case toplevel_only::no:
            for (auto it = env_data_.rbegin(); it != env_data_.rend(); ++it) {
                if (auto found = it->find(final_key); found != it->end()) {
                    AD_LOG(acmacs::log::settings, "env found \"{}\": {}", found->first, found->second);
                    return substitute(found->second);
                }
            }
            break;
        case toplevel_only::yes:
            if (auto found = env_data_.back().find(final_key); found != env_data_.back().end()) {
                AD_LOG(acmacs::log::settings, "env found (toplevel) \"{}\": {}", found->first, found->second);
                return substitute(found->second);
            }
            break;
    }
    AD_LOG(acmacs::log::settings, "not found in env \"{}\" (\"{}\") toplevel_only:{}", final_key, key, a_toplevel_only == toplevel_only::yes ? "yes" : "no");
    return rjson::v3::const_null;

} // acmacs::settings::v3::detail::Environment::get

// ----------------------------------------------------------------------

bool acmacs::settings::v3::detail::Environment::replace(std::string_view key, rjson::v3::value&& val) // returns if replaced
{
    for (auto it = env_data_.rbegin(); it != env_data_.rend(); ++it) {
        if (auto found = it->find(key); found != it->end()) {
            found->second = std::move(val);
            return true;
        }
    }
    return false;

} // acmacs::settings::v3::detail::Environment::replace

// ----------------------------------------------------------------------

std::string acmacs::settings::v3::detail::Environment::substitute(std::string_view text) const
{
#include "acmacs-base/global-constructors-push.hh"
    static const std::regex re{"\\{([^\\{}]+)\\}"};
#include "acmacs-base/diagnostics-pop.hh"

    std::string result{text};
    std::smatch m1;
    AD_LOG(acmacs::log::settings, "env substitute s->s \"{}\"", text);
    AD_LOG_INDENT;
    for (ssize_t start{0}; static_cast<size_t>(start) < result.size() && std::regex_search(std::next(std::cbegin(result), start), std::cend(result), m1, re);) {
        const auto replacement_start = start + m1.position();
        if (const auto& found1 = get(m1.str(1)); !found1.is_null()) {
            result.erase(static_cast<size_t>(replacement_start), static_cast<size_t>(m1.length()));
            result.insert(static_cast<size_t>(replacement_start), found1.as_string());
            AD_LOG(acmacs::log::settings, "env substituted -> \"{}\"", result);
        }
        else // no replacement found, leave as is
            start = replacement_start + m1.length();
    }
    AD_LOG(acmacs::log::settings, "env substituted s->s \"{}\" -> \"{}\"", text, result);
    return result;

} // acmacs::settings::v3::detail::Environment::substitute_to_string

// ----------------------------------------------------------------------

const rjson::v3::value& acmacs::settings::v3::detail::Environment::substitute(std::string_view text, const rjson::v3::value& source) const
{
    if (text.size() > 1 && text[0] == '$')
        return get(text.substr(1));

    if (std::string result = substitute(text); result != text) {
#include "acmacs-base/global-constructors-push.hh"
        static thread_local rjson::v3::value substituted;
#include "acmacs-base/diagnostics-pop.hh"
        AD_LOG(acmacs::log::settings, "env substitute static old: {}", substituted);
        substituted = rjson::v3::detail::string(rjson::v3::detail::string::with_content, std::move(result));
        AD_LOG(acmacs::log::settings, "env substitute static new: {}", substituted);
        return substituted;
    }
    else {
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

std::string acmacs::settings::v3::detail::Environment::format(std::string_view indent) const
{
    fmt::memory_buffer out;
    for (auto [level, entries] : acmacs::enumerate(env_data_)) {
        for (const auto& entry : entries)
            fmt::format_to_mb(out, "{}\"{}\": {} <{}>\n", indent, entry.first, entry.second, level);
    }
    return fmt::to_string(out);

} // acmacs::settings::v3::detail::Environment::format

// ----------------------------------------------------------------------

std::string acmacs::settings::v3::detail::Environment::format_toplevel() const
{
    fmt::memory_buffer out;
    fmt::format_to_mb(out, "{{");
    bool first { true };
    for (const auto& entry : env_data_.back()) {
        if (first)
            first = false;
        else
            fmt::format_to_mb(out, ", ");
        fmt::format_to_mb(out, "\"{}\": {}", entry.first, entry.second);
    }
    fmt::format_to_mb(out, "}}");
    return fmt::to_string(out);

} // acmacs::settings::v3::detail::Environment::format_toplevel

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
