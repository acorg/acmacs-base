#include "acmacs-base/settings-v3.hh"
#include "acmacs-base/settings-v3-env.hh"

// ----------------------------------------------------------------------

namespace
{
    struct raii_true
    {
        constexpr raii_true(bool& val) : val_{val} { val_ = true; }
        ~raii_true() { val_ = false; }
        bool& val_;
    };
}

// ----------------------------------------------------------------------

acmacs::settings::v3::Data::Data()
    : loaded_data_{std::make_unique<detail::LoadedDataFiles>()}, environment_{std::make_unique<detail::Environment>()}
{

} // acmacs::settings::v3::Data::Data

// ----------------------------------------------------------------------

acmacs::settings::v3::Data::~Data()
{
} // acmacs::settings::v3::Data::~Data

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::apply(std::string_view name)
{
    if (name.empty())
        throw error{AD_FORMAT("cannot apply command with an empty name")};
    if (name.front() != '?') { // not commented out
        const auto substituted_name = environment_->substitute_to_string(name);
        AD_LOG(acmacs::log::settings, "apply \"{}\" <-- \"{}\"", substituted_name, name);
        AD_LOG_INDENT;
        if (const auto& val_from_data = get(substituted_name); !val_from_data.is_null()) {
            apply(val_from_data);
        }
        else if (!apply_built_in(substituted_name)) {
            throw error{AD_FORMAT("settings entry not found: \"{}\" (not substituted: \"{}\")", substituted_name, name)};
        }
    }

} // acmacs::settings::v3::Data::apply

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::apply(const rjson::v3::value& entry)
{
    try {
        AD_LOG(acmacs::log::settings, "apply {}", entry);
        for (const auto& sub_entry : entry.array()) {
            sub_entry.visit([this, &sub_entry]<typename T>(const T& sub_entry_val) {
                if constexpr (std::is_same_v<T, rjson::v3::detail::string>)
                    apply(sub_entry_val.template to<std::string_view>());
                else if constexpr (std::is_same_v<T, rjson::v3::detail::object>)
                    push_and_apply(sub_entry_val);
                else if constexpr (std::is_same_v<T, rjson::v3::detail::null>)
                    ; // commented out item (using #), do nothing
                else
                    throw error{AD_FORMAT("cannot apply: {} (string or object expected)\n", sub_entry)};
            });
        }
    }
    catch (rjson::v3::value_type_mismatch& err) {
        throw error{AD_FORMAT("cannot apply: {} : {}\n", err, entry)};
    }
    catch (std::exception& err) {
        throw error{fmt::format("{}\n    on applying {}", err, entry)};
    }

} // acmacs::settings::v3::Data::apply

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::push_and_apply(const rjson::v3::detail::object& entry)
{
    class Subenvironment
    {
      public:
        Subenvironment(detail::Environment& env, bool push) : env_{env}, push_{push}
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
        detail::Environment& env_;
        const bool push_;
    };

    using namespace std::string_view_literals;
    AD_LOG_INDENT;
    AD_LOG(acmacs::log::settings, "push_and_apply {}", entry);
    try {
        if (const auto& command_v = entry["N"sv]; !command_v.is_null()) {
            const auto command{command_v.to<std::string_view>()};
            AD_LOG(acmacs::log::settings, "push_and_apply command {} -> {}", command_v, command);
            Subenvironment sub_env(*environment_, command != "set");
            for (const auto& [key, val] : entry) {
                if (key != "N"sv) {
                    // AD_LOG(acmacs::log::settings, "environment_.add key:{} val:{}", key, val);
                    environment_->add(key, val);
                }
            }
            if (command != "set")
                apply(command);
            else if (warn_if_set_used_)
                AD_WARNING("\"set\" command has no effect (used inside \"if\" or \"for-each\"?): {}", entry);
        }
        else if (!entry["?N"sv].is_null() || !entry["? N"sv].is_null()) {
            // command is commented out
        }
        else
            throw error(AD_FORMAT("cannot apply (no \"N\"): {}\n", entry));
    }
    catch (rjson::v3::value_type_mismatch& err) {
        throw error(AD_FORMAT("cannot apply: {} : {}\n", err, entry));
    }

} // acmacs::settings::v3::Data::push_and_apply

// ----------------------------------------------------------------------

bool acmacs::settings::v3::Data::apply_built_in(std::string_view name) // returns true if built-in command with that name found and applied
{
    using namespace std::string_view_literals;
    AD_LOG(acmacs::log::settings, "base::apply_built_in \"{}\"", name);
    AD_LOG_INDENT;
    try {
        if (name == "if"sv) {
            apply_if();
            return true;
        }
        else if (name == "for-each"sv) {
            apply_for_each();
            return true;
        }
        else if (name == "-print-environment"sv) {
            AD_INFO("{}:\n{}", name, environment_->format("    "sv));
            return true;
        }
        return false;
    }
    catch (std::exception& err) {
        throw error{AD_FORMAT("cannot apply \"{}\": {}", name, err)};
    }

} // acmacs::settings::v3::Data::apply_built_in

// ----------------------------------------------------------------------

const rjson::v3::value& acmacs::settings::v3::Data::get(std::string_view name) const
{
    return loaded_data_->get(name);

} // acmacs::settings::v3::Data::get

// ----------------------------------------------------------------------

std::string acmacs::settings::v3::Data::substitute_to_string(std::string_view source) const
{
    return environment_->substitute_to_string(source);

} // acmacs::settings::v3::Data::substitute_to_string

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::apply_if()
{
    raii_true warn_if_set_used{warn_if_set_used_};
///    if (const auto& condition_clause = getenv("condition", toplevel_only::yes); eval_condition(condition_clause)) {
///        if (const auto& then_clause = getenv("then", toplevel_only::yes); !then_clause.is_null()) {
///            AD_LOG(acmacs::log::settings, "if then {}", then_clause);
///            if (!then_clause.is_array())
///                throw error{AD_FORMAT("\"then\" clause must be array")};
///            apply(then_clause);
///        }
///    }
///    else {
///        if (const auto& else_clause = getenv("else", toplevel_only::yes); !else_clause.is_null()) {
///            AD_LOG(acmacs::log::settings, "if else {}", else_clause);
///            if (!else_clause.is_array())
///                throw error{AD_FORMAT("\"else\" clause must be array")};
///            apply(else_clause);
///        }
///    }

} // acmacs::settings::v3::Data::apply_if

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::apply_for_each()
{
    raii_true warn_if_set_used{warn_if_set_used_};

    using namespace std::string_view_literals;

///    const auto var_name = getenv_or("var"sv, "name"sv);
///    const auto& values_clause = getenv("values"sv, toplevel_only::yes);
///    if (!values_clause.is_array())
///        throw error{AD_FORMAT("\"values\" clause must be array")};
///    const auto& do_clause = getenv("do"sv, toplevel_only::yes);
///    if (!do_clause.is_array())
///        throw error{AD_FORMAT("\"do\" clause must be array")};
///    for (const auto& val : values_clause.array()) {
///        environment_.push();
///        environment_.add(var_name, val);
///        apply(do_clause);
///        environment_.pop();
///    }

} // acmacs::settings::v3::Data::apply_for_each

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::load(std::string_view filename)
{
    using namespace std::string_view_literals;
    AD_LOG(acmacs::log::settings, "loading {}", filename);
    loaded_data_->load(filename);
// apply_top("init"sv);

} // acmacs::settings::v3::Data::load

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::load(const std::vector<std::string_view>& filenames)
{

} // acmacs::settings::v3::Data::load

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::load_from_conf(const std::vector<std::string_view>& filenames) // load from ${ACMACSD_ROOT}/share/conf dir
{

} // acmacs::settings::v3::Data::load_from_conf

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::set_defines(const std::vector<std::string_view>& defines)
{

} // acmacs::settings::v3::Data::set_defines

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::reload() // reset environament, re-load previously loaded files, apply "init" in loaded files
{

} // acmacs::settings::v3::Data::reload

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
