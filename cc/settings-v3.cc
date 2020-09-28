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

void acmacs::settings::v3::Data::apply(std::string_view name, toplevel_only tlo)
{
    using namespace std::string_view_literals;

    if (name.empty())
        throw error{AD_FORMAT("cannot apply command with an empty name")};
    if (name.front() != '?') { // not commented out
        const auto substituted_name = environment_->substitute(name);
        AD_LOG(acmacs::log::settings, "apply{} \"{}\" <-- \"{}\"", tlo == toplevel_only::yes ? " (top level)"sv: ""sv, substituted_name, name);
        AD_LOG_INDENT;
        if (const auto& val_from_data = get(substituted_name, tlo); !val_from_data.is_null()) {
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

const rjson::v3::value& acmacs::settings::v3::Data::get(std::string_view name, toplevel_only tlo) const
{
    switch (tlo) {
        case toplevel_only::yes:
            return loaded_data_->top(name);
        case toplevel_only::no:
            return loaded_data_->get(name);
    }

} // acmacs::settings::v3::Data::get

// ----------------------------------------------------------------------

bool acmacs::settings::v3::Data::eval_condition(const rjson::v3::value& condition) const
{
    using namespace std::string_view_literals;
    try {
        return condition.visit([this, &condition]<typename Arg>(const Arg& arg) -> bool {
            if constexpr (std::is_same_v<Arg, rjson::v3::detail::null>)
                return false;
            else if constexpr (std::is_same_v<Arg, rjson::v3::detail::boolean>)
                return arg.template to<bool>();
            else if constexpr (std::is_same_v<Arg, rjson::v3::detail::number>)
                return !float_zero(arg.template to<double>());
            else if constexpr (std::is_same_v<Arg, bool>)
                return arg;
            else if constexpr (std::is_same_v<Arg, rjson::v3::detail::object>) {
                if (arg.size() != 1)
                    throw error{AD_FORMAT("object must have exactly one key: {}, object size: {}", condition, arg.size())};
                if (const auto& and_clause = arg["and"sv]; !and_clause.is_null())
                    return eval_and(and_clause);
                else if (const auto& empty_clause = arg["empty"sv]; !empty_clause.is_null())
                    return eval_empty(empty_clause, true);
                else if (const auto& not_empty_clause = arg["not-empty"sv]; !not_empty_clause.is_null())
                    return eval_empty(not_empty_clause, false);
                else if (const auto& equal_clause = arg["equal"sv]; !equal_clause.is_null())
                    return eval_equal(equal_clause);
                else if (const auto& not_clause = arg["not"sv]; !not_clause.is_null())
                    return eval_not(not_clause);
                else if (const auto& not_equal_clause = arg["not-equal"sv]; !not_equal_clause.is_null())
                    return eval_not_equal(not_equal_clause);
                else if (const auto& or_clause = arg["or"sv]; !or_clause.is_null())
                    return eval_or(or_clause);
                else
                    throw error{AD_FORMAT("unrecognized clause: {}", condition)};
            }
            else if constexpr (std::is_same_v<Arg, rjson::v3::detail::string>) {
                const auto arg_sv = arg.template to<std::string_view>();
                if (const rjson::v3::value& substituted = environment().substitute(arg_sv, condition); substituted.is_null() || (substituted.is_string() && substituted.to<std::string_view>() == arg_sv))
                    throw error{AD_FORMAT("unsupported value type: {}", condition)};
                else
                    return eval_condition(substituted);
            }
            else // if constexpr (std::is_same_v<Arg, rjson::v3::detail::array>)
                throw error{AD_FORMAT("unsupported value type: {}", condition)};
        });
    }
    catch (std::exception& err) {
        throw error(AD_FORMAT("cannot eval condition: {} -- condition: {}\n", err, condition));
    }

} // acmacs::settings::v3::Data::eval_condition

// ----------------------------------------------------------------------

bool acmacs::settings::v3::Data::eval_and(const rjson::v3::value& condition) const
{
    if (condition.empty()) {
        AD_WARNING("empty and clause evaluates to false");
        return false;
    }
    const auto& arr = condition.array();
    return std::all_of(std::begin(arr), std::end(arr), [this](const rjson::v3::value& sub_condition) { return eval_condition(sub_condition); });

} // acmacs::settings::v3::Data::eval_and

// ----------------------------------------------------------------------

bool acmacs::settings::v3::Data::eval_or(const rjson::v3::value& condition) const
{
    if (condition.empty()) {
        AD_WARNING("empty or clause evaluates to false");
        return false;
    }
    const auto& arr = condition.array();
    return std::any_of(std::begin(arr), std::end(arr), [this](const rjson::v3::value& sub_condition) { return eval_condition(sub_condition); });

} // acmacs::settings::v3::Data::eval_or

// ----------------------------------------------------------------------

bool acmacs::settings::v3::Data::eval_empty(const rjson::v3::value& condition, bool true_if_empty) const
{
    try {
        return environment().substitute(condition).visit([true_if_empty]<typename Arg>(const Arg& arg) -> bool {
            if constexpr (std::is_same_v<Arg, rjson::v3::detail::null>)
                return true_if_empty;
            else if constexpr (std::is_same_v<Arg, rjson::v3::detail::string>)
                return arg.empty() == true_if_empty;
            else
                throw error{AD_FORMAT("unsupported value type")};
        });
    }
    catch (std::exception& err) {
        throw error(AD_FORMAT("cannot eval condition: {} -- condition: {}\n", err, condition));
    }

} // acmacs::settings::v3::Data::eval_empty

// ----------------------------------------------------------------------

bool acmacs::settings::v3::Data::eval_equal(const rjson::v3::value& condition) const
{
    if (condition.empty()) {
        AD_WARNING("empty equal clause evaluates to false");
        return false;
    }
    if (!condition.is_array() || condition.size() < 2) {
        AD_WARNING("equal clause condition must be an array with 2 or more elements: {}, evaluates to false", condition);
        return false;
    }

    const auto& first = environment().substitute(condition[0]);
    for (size_t index = 1; index < condition.size(); ++index) {
        if (!(environment().substitute(condition[index]) == first))
            return false;
    }
    return true;

} // acmacs::settings::v3::Data::eval_equal

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::apply_if()
{
    raii_true warn_if_set_used{warn_if_set_used_};
    if (const auto& condition_clause = environment().get("condition", detail::toplevel_only::yes); eval_condition(condition_clause)) {
        if (const auto& then_clause = environment().get("then", detail::toplevel_only::yes); !then_clause.is_null()) {
            AD_LOG(acmacs::log::settings, "if then {}", then_clause);
            if (!then_clause.is_array())
                throw error{AD_FORMAT("\"then\" clause must be array")};
            apply(then_clause);
        }
    }
    else {
        if (const auto& else_clause = environment().get("else", detail::toplevel_only::yes); !else_clause.is_null()) {
            AD_LOG(acmacs::log::settings, "if else {}", else_clause);
            if (!else_clause.is_array())
                throw error{AD_FORMAT("\"else\" clause must be array")};
            apply(else_clause);
        }
    }

} // acmacs::settings::v3::Data::apply_if

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::apply_for_each()
{
    raii_true warn_if_set_used{warn_if_set_used_};

    using namespace std::string_view_literals;

    const auto var_name = environment().get_or("var"sv, "name"sv);
    const auto& values_clause = environment().get("values"sv, detail::toplevel_only::yes);
    if (!values_clause.is_array())
        throw error{AD_FORMAT("\"values\" clause must be array")};
    const auto& do_clause = environment().get("do"sv, detail::toplevel_only::yes);
    if (!do_clause.is_array())
        throw error{AD_FORMAT("\"do\" clause must be array")};
    for (const auto& val : values_clause.array()) {
        environment().push();
        environment().add(var_name, val);
        apply(do_clause);
        environment().pop();
    }

} // acmacs::settings::v3::Data::apply_for_each

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::load(std::string_view filename)
{
    using namespace std::string_view_literals;
    AD_LOG(acmacs::log::settings, "loading {}", filename);
    loaded_data_->load(filename);
    apply("init"sv, toplevel_only::yes);

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
