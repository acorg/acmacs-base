#include "acmacs-base/settings.hh"
#include "acmacs-base/enumerate.hh"

// ----------------------------------------------------------------------

namespace acmacs::settings::inline v2
{
    class Subenvironment
    {
      public:
        Subenvironment(Settings::Environment& env, bool push) : env_{env}, push_{push}
        {
            if (push_)
                env_.push();
        }
        ~Subenvironment()
        {
            if (push_)
                env_.pop();
        }

      private:
        Settings::Environment& env_;
        const bool push_;
    };
} // namespace acmacs::settings::inlinev2

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
    if (name.empty())
        throw error("cannot apply command with an empty name");
    if (name.front() != '?') { // not commented out
        if (const auto& val1 = environment_.get(name); !val1.is_const_null()) {
            apply(val1);
        }
        else if (const auto& val2 = get(name); !val2.is_const_null())
            apply(val2);
        else if (!apply_built_in(name))
            throw error(fmt::format("settings entry not found: \"{}\"", name));
    }

} // acmacs::settings::v2::Settings::apply

// ----------------------------------------------------------------------

bool acmacs::settings::v2::Settings::apply_built_in(std::string_view name) const
{
    if (name == ":print-all-environment") {
        environment_.print();
        return true;
    }
    else if (name == ":print-environment-key-value") {
        environment_.print_key_value();
        return true;
    }
    return false;

} // acmacs::settings::v2::Settings::apply_built_in

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
            Subenvironment sub_env(environment_, command != "set");
            entry.for_each([this](const std::string& key, const rjson::value& val) {
                if (key != "N")
                    environment_.add(key, val);
            });
            if (command != "set")
                apply(command);
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

const rjson::value& acmacs::settings::v2::Settings::Environment::get(std::string_view key) const
{
    for (auto it = data_.rbegin(); it != data_.rend(); ++it) {
        if (auto found = it->find(key); found != it->end())
            return found->second;
    }
    return rjson::ConstNull;

} // acmacs::settings::v2::Settings::Environment::get

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::Environment::print() const
{
    fmt::print("INFO: Settings::Environment {}\n", data_.size());
    for (auto [level, entries] : acmacs::enumerate(data_)) {
        for (const auto& entry : entries)
            fmt::print("    {} \"{}\": {}\n", level, entry.first, entry.second);
    }

} // acmacs::settings::v2::Settings::Environment::print

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::Environment::print_key_value() const
{
    fmt::print("INFO: Settings::Environment::print_key_value {}: {}\n", get("key"), get("value"));

} // acmacs::settings::v2::Settings::Environment::print_value

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
