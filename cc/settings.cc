#include "acmacs-base/settings.hh"

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::load(const std::vector<std::string_view>& filenames)
{
    for (const auto& filename : filenames)
        data_.push_back(rjson::parse_file(filename, rjson::remove_comments::no));
    std::reverse(std::begin(data_), std::end(data_));

} // acmacs::settings::v2::Settings::load

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::apply(std::string_view name) const
{
    if (const auto& val = get(name); !val.is_const_null())
        apply(val);
    else
        throw error(fmt::format("settings entry not found: \"{}\"", name));

} // acmacs::settings::v2::Settings::apply

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::apply(const rjson::value& entry) const
{
    try {
        rjson::for_each(entry, [this](const rjson::value& sub_entry) {
            std::visit(
                [this](auto&& sub_entry_val) {
                    using T = std::decay_t<decltype(sub_entry_val)>;
                    if constexpr (std::is_same_v<T, std::string>)
                        this->apply(std::string_view{sub_entry_val});
                    else if constexpr (std::is_same_v<T, rjson::object>)
                        this->push_and_apply(sub_entry_val);
                    else
                        throw error(fmt::format("cannot apply: {}\n", sub_entry_val));
                },
                sub_entry.val_());
        });
    }
    catch (rjson::value_type_mismatch& err) {
        throw error(fmt::format("cannot apply: {} : {}\n", err, entry));
    }

} // acmacs::settings::v2::Settings::apply

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::push_and_apply(const rjson::object& entry) const
{
    try {
        if (const auto& command_v = entry.get("N"); !command_v.is_const_null()) {
            const std::string_view command = command_v;
            if ( command != "set")
                this->environment_.emplace();
            entry.for_each([this](const std::string& key, const rjson::value& val) {
                if (key != "N")
                    this->environment_.top().emplace_or_replace(key, val);
            });
            if (command != "set") {
                apply(command);
                this->environment_.pop();
            }
        }
        else if (const auto& commented_command_v = entry.get("?N"); !commented_command_v.is_const_null()) {
            // command is commented out
        }
        else
            throw error(fmt::format("cannot apply: {}\n", entry));
    }
    catch (rjson::value_type_mismatch& err) {
        throw error(fmt::format("cannot apply: {} : {}\n", err, entry));
    }


} // acmacs::settings::v2::Settings::push_and_apply

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
