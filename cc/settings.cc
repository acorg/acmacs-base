#include <regex>

#include "acmacs-base/acmacsd.hh"
#include "acmacs-base/filesystem.hh"
#include "acmacs-base/settings.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-base/string-from-chars.hh"

// ----------------------------------------------------------------------
// Environment
// ----------------------------------------------------------------------

const rjson::v3::value& acmacs::settings::v2::Settings::Environment::substitute_to_value(std::string_view source) const noexcept
{
    AD_LOG(acmacs::log::settings, "substitute to value \"{}\"", source);
    AD_LOG_INDENT;
    return std::visit(
        []<typename Cont>(Cont cont) -> const rjson::v3::value& {
            if constexpr (std::is_same_v<Cont, const rjson::v3::value*>)
                return *cont;
            else
                return rjson::v3::const_null; // partial or no substitution
        },
        substitute(source));

} // acmacs::settings::v2::Settings::Environment::substitute_to_value

// ----------------------------------------------------------------------

std::string acmacs::settings::v2::Settings::Environment::substitute_to_string(std::string_view source) const noexcept
{
    return std::visit(
        [source]<typename Cont>(Cont cont) -> std::string {
            if constexpr (std::is_same_v<Cont, const rjson::v3::value*>) {
                AD_LOG(acmacs::log::settings, "substitute to string \"{}\" -> {}", source, *cont);
                return std::string{cont->template to<std::string_view>()};
            }
            else if constexpr (std::is_same_v<Cont, no_substitution_request>) {
                AD_LOG(acmacs::log::settings, "no substitution request in \"{}\"", source);
                return std::string{source};
            }
            else {
                AD_LOG(acmacs::log::settings, "substitute to string \"{}\" -> \"{}\"", source, cont);
                return cont;
            }
        },
        substitute(source));

} // acmacs::settings::v2::Settings::Environment::substitute_to_string

// ----------------------------------------------------------------------

acmacs::settings::v2::Settings::substitute_result_t acmacs::settings::v2::Settings::Environment::substitute(std::string_view source) const
{
#include "acmacs-base/global-constructors-push.hh"
    static const std::regex re{"\\{([^\\}]+)\\}"};
#include "acmacs-base/diagnostics-pop.hh"

    AD_ASSERT(!source.empty(), "Environment::substitute called with empty string");
    AD_LOG(acmacs::log::settings, "substitute in string \"{}\"", source);

    if (std::cmatch m1; std::regex_search(std::begin(source), std::end(source), m1, re)) {
        if (const auto& found1 = get(m1.str(1), toplevel_only::no); m1.position(0) == 0 && static_cast<size_t>(m1.length(0)) == source.size()) {
            if (found1.is_null()) {
                AD_LOG(acmacs::log::settings, "substitute in string \"{}\" -> null", source);
                return &rjson::v3::const_null; // entire source matched and no substitution found, return null (necessary to support clades with "date": "{date-recent}" overriding for mapi
            }
            else {
                AD_LOG(acmacs::log::settings, "substitute in string \"{}\" -> {}", source, found1);
                return &found1;
            }
        }
        else { // not whole_source_matched
            const auto replace = [](const auto& src, size_t prefix, const rjson::v3::value& infix, size_t suffix) {
                if (infix.is_null())
                    return fmt::format("{}{}", src.substr(0, prefix), src.substr(suffix));
                else
                    return fmt::format("{}{}{}", src.substr(0, prefix), infix.as_string(), src.substr(suffix));
            };
            const auto replaced = replace(source, static_cast<size_t>(m1.position(0)), found1, static_cast<size_t>(m1.position(0) + m1.length(0)));
            AD_LOG(acmacs::log::settings, "substitute in string \"{}\" -> \"{}\" (partial substitution)", source, replaced);
            return std::visit(
                []<typename Cont>(Cont cont) -> substitute_result_t {
                    if constexpr (std::is_same_v<Cont, no_substitution_request>)
                        return *cont; // return string and not no_substitution_request!
                    else
                        return cont;
                },
                substitute(replaced));
        }
    }
    else { // no substitution requests
        AD_LOG(acmacs::log::settings, "substitute in string \"{}\" -> \"{}\" (no substitution requests)", source, source);
        return no_substitution_request{source};
    }

} // acmacs::settings::v2::Settings::Environment::substitute

// ----------------------------------------------------------------------

const rjson::v3::value& acmacs::settings::v2::Settings::Environment::get(std::string_view key, toplevel_only a_toplevel_only) const
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

} // acmacs::settings::v2::Settings::Environment::get

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::Environment::print() const
{
    AD_INFO("Settings::Environment {}", env_data_.size());
    for (auto [level, entries] : acmacs::enumerate(env_data_)) {
        for (const auto& entry : entries) {
            if (entry.second.is_string()) {
                if (entry.second.empty())
                    AD_INFO("    {} \"{}\": \"\"", level, entry.first);
                else
                    AD_INFO("    {} \"{}\": \"{}\"", level, entry.first, substitute_to_string(entry.second.to<std::string_view>()));
            }
            else
                AD_INFO("    {} \"{}\": {}", level, entry.first, entry.second);
        }
    }

} // acmacs::settings::v2::Settings::Environment::print

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::Environment::print_key_value() const
{
    const auto& val = get("value", toplevel_only::no);
    if (val.is_string()) {
        if (const auto& substituted = substitute_to_value(val.to<std::string_view>()); substituted.is_null())
            fmt::print("{}: \"{}\"\n", get("key", toplevel_only::no), substitute_to_string(val.to<std::string_view>()));
        else
            fmt::print("{}: {}\n", get("key", toplevel_only::no), substituted);
    }
    else
        fmt::print("{}: {}\n", get("key", toplevel_only::no), val);

} // acmacs::settings::v2::Settings::Environment::print_value

// ----------------------------------------------------------------------
// Subenvironment
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

    struct raii_true
    {
        constexpr raii_true(bool& val) : val_{val} { val_ = true; }
        ~raii_true() { val_ = false; }
        bool& val_;
    };

} // namespace acmacs::settings::inlinev2

// ----------------------------------------------------------------------
// Settings
// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::apply(std::string_view name)
{
    if (name.empty())
        throw error{AD_FORMAT("cannot apply command with an empty name")};
    if (name.front() != '?') { // not commented out
        AD_LOG(acmacs::log::settings, "apply \"{}\"", name);
        AD_LOG_INDENT;
        const auto substituted_name = environment_.substitute_to_string(name);
        AD_LOG(acmacs::log::settings, "apply substituted \"{}\"", substituted_name);
        if (const auto& val1 = environment_.get(substituted_name, toplevel_only::no); !val1.is_null()) {
            AD_LOG(acmacs::log::settings, "apply val1 {}", val1);
            apply(val1);
        }
        else if (const auto& val2 = get(substituted_name); !val2.is_null()) {
            AD_LOG(acmacs::log::settings, "apply val2 {}", val1);
            apply(val2);
        }
        else if (!apply_built_in(substituted_name)) {
            // loaded_data_.report();
            throw error{AD_FORMAT("settings entry not found: \"{}\" (not substituted: \"{}\")", substituted_name, name)};
        }
    }

} // acmacs::settings::v2::Settings::apply

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::apply_first(const std::vector<std::string_view>& names, throw_if_nothing_applied tina)
{
    bool applied{false};
    for (const auto& to_apply : names) {
        if (!get(to_apply).is_null()) {
            AD_LOG(acmacs::log::settings, "applying {}", to_apply);
            apply(to_apply);
            applied = true;
            break;
        }
    }
    if (!applied) {
        AD_LOG(acmacs::log::settings, "apply_first: NOT applied {}", names);
        if (tina == throw_if_nothing_applied::yes)
            throw std::runtime_error{fmt::format("neither of {} found in settings", names)};
    }

} // acmacs::settings::v2::Settings::apply_first

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::apply_top(std::string_view name)
{
    AD_LOG(acmacs::log::settings, "apply_top \"{}\"", name);
    if (name.empty())
        throw error{AD_FORMAT("cannot apply command with an empty name")};
    if (name.front() != '?') { // not commented out
        const auto substituted_name = environment_.substitute_to_string(name);
        if (const auto& val = loaded_data_.get_top(substituted_name); !val.is_null())
            apply(val);
    }

} // acmacs::settings::v2::Settings::apply_top

// ----------------------------------------------------------------------

bool acmacs::settings::v2::Settings::apply_built_in(std::string_view name)
{
    AD_LOG(acmacs::log::settings, "base::apply_built_in \"{}\"", name);
    AD_LOG_INDENT;
    try {
        if (name == "if") {
            apply_if();
            return true;
        }
        else if (name == ":print-all-environment") {
            environment_.print();
            return true;
        }
        else if (name == ":print-environment-key-value") {
            environment_.print_key_value();
            return true;
        }
        return false;
    }
    catch (std::exception& err) {
        throw error{AD_FORMAT("cannot apply \"{}\": {}", name, err)};
    }

} // acmacs::settings::v2::Settings::apply_built_in

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::apply(const rjson::v3::value& entry)
{
    try {
        AD_LOG(acmacs::log::settings, "apply {}", entry);
        for (const auto& sub_entry : entry.array()) {
            // AD_DEBUG("apply array element {}", sub_entry);
            sub_entry.visit([this, &sub_entry]<typename T>(const T& sub_entry_val) {
                if constexpr (std::is_same_v<T, rjson::v3::detail::string>)
                    this->apply(sub_entry_val.template to<std::string_view>());
                else if constexpr (std::is_same_v<T, rjson::v3::detail::object>)
                    this->push_and_apply(sub_entry_val);
                else if constexpr (std::is_same_v<T, rjson::v3::detail::null>)
                    ; // commented out item (using #), do nothing
                else
                    throw error{AD_FORMAT("cannot apply: {}\n", sub_entry)};
            });
        }
    }
    catch (rjson::v3::value_type_mismatch& err) {
        throw error{AD_FORMAT("cannot apply: {} : {}\n", err, entry)};
    }
    catch (std::exception& err) {
        throw error{fmt::format("{}\n    on applying {}", err.what(), entry)};
    }

} // acmacs::settings::v2::Settings::apply

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::push_and_apply(const rjson::v3::detail::object& entry)
{
    using namespace std::string_view_literals;
    AD_LOG_INDENT;
    AD_LOG(acmacs::log::settings, "push_and_apply {}", entry);
    try {
        if (const auto& command_v = entry["N"sv]; !command_v.is_null()) {
            const auto command{command_v.to<std::string_view>()};
            AD_LOG(acmacs::log::settings, "push_and_apply command {} -> {}", command_v, command);
            Subenvironment sub_env(environment_, command != "set");
            for (const auto& [key, val] : entry) {
                if (key != "N"sv) {
                    // AD_LOG(acmacs::log::settings, "environment_.add key:{} val:{}", key, val);
                    environment_.add(key, val);
                }
            }
            if (command != "set")
                apply(command);
            else if (warn_if_set_used_)
                AD_WARNING("\"set\" command has no effect (used inside \"if\"?): {}", entry);
        }
        else if (!entry["?N"sv].is_null() || !entry["? N"sv].is_null()) {
            // command is commented out
        }
        else
            throw error(AD_FORMAT("cannot apply: {}\n", entry));
    }
    catch (rjson::v3::value_type_mismatch& err) {
        throw error(AD_FORMAT("cannot apply: {} : {}\n", err, entry));
    }

} // acmacs::settings::v2::Settings::push_and_apply

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::apply_if()
{
    raii_true warn_if_set_used{warn_if_set_used_};
    if (const auto& condition_clause = getenv("condition", toplevel_only::yes); eval_condition(condition_clause)) {
        if (const auto& then_clause = getenv("then", toplevel_only::yes); !then_clause.is_null()) {
            AD_LOG(acmacs::log::settings, "if then {}", then_clause);
            if (!then_clause.is_array())
                throw error{AD_FORMAT("\"then\" clause must be array")};
            apply(then_clause);
        }
    }
    else {
        if (const auto& else_clause = getenv("else", toplevel_only::yes); !else_clause.is_null()) {
            AD_LOG(acmacs::log::settings, "if else {}", else_clause);
            if (!else_clause.is_array())
                throw error{AD_FORMAT("\"else\" clause must be array")};
            apply(else_clause);
        }
    }

} // acmacs::settings::v2::Settings::apply_if

// ----------------------------------------------------------------------

bool acmacs::settings::v2::Settings::eval_condition(const rjson::v3::value& condition) const
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
                if (const rjson::v3::value& substituted = environment_.substitute_to_value(arg_sv); substituted.is_null() || (substituted.is_string() && substituted.to<std::string_view>() == arg_sv))
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

} // acmacs::settings::v2::Settings::eval_condition

// ----------------------------------------------------------------------

const rjson::v3::value& acmacs::settings::v2::Settings::getenv(std::string_view key, toplevel_only a_toplevel_only, if_no_substitution_found ifnsf, throw_if_partial_substitution tips) const
{
    AD_LOG(acmacs::log::settings, "getenv \"{}\"", key);
    if (const auto& val = environment_.get(environment_.substitute_to_string(key), a_toplevel_only); val.is_string()) {
        const rjson::v3::value* orig = &val;
        for (size_t num_subst = 0; num_subst < 10; ++num_subst) {
            if (orig->empty())
                return *orig;

            const rjson::v3::value& substituted = std::visit(
                [&orig, ifnsf, tips]<typename Cont>(Cont cont) -> const rjson::v3::value& {
                    if constexpr (std::is_same_v<Cont, const rjson::v3::value*>) {
                        if (cont->is_null()) {
                            switch (ifnsf) {
                              case if_no_substitution_found::leave_as_is:
                                  return *orig;
                                  break;
                              case if_no_substitution_found::null:
                                  return rjson::v3::const_null;
                                  break;
                              case if_no_substitution_found::empty:
                                  return rjson::v3::const_empty_string;
                                  break;
                            }
                        }
                        else
                            return *cont;
                    }
                    else if constexpr (std::is_same_v<Cont, no_substitution_request>) {
                        return *orig;
                    }
                    else { // no subst or partial subst
                        if (cont != orig->to<std::string_view>() && tips == throw_if_partial_substitution::yes) {
                            AD_ERROR("getenv: partial subst {} -> \"{}\" (aborting!)", *orig, cont);
                            abort();
                        }
                        else
                            return rjson::v3::const_null;
                    }
                },
                environment_.substitute(orig->to<std::string_view>()));

            AD_LOG(acmacs::log::settings, "getenv substituted {} -> {}", *orig, substituted);
            if (substituted.is_string() && !(*orig == substituted))
                orig = &substituted;
            // else if (substituted.is_null())
            //     return *orig;
            else
                return substituted;
        }
        throw error(AD_FORMAT("Settings::getenv: too many substitutions in {}", val));
    }
    else
        return val;

} // acmacs::settings::v2::Settings::getenv

// ----------------------------------------------------------------------

const rjson::v3::value& acmacs::settings::v2::Settings::getenv(std::string_view key1, std::string_view key2, toplevel_only a_toplevel_only, if_no_substitution_found ifnsf, throw_if_partial_substitution tips) const
{
    if (const auto& val = getenv(key1, a_toplevel_only, ifnsf, tips); !val.is_null())
        return val;
    else
        return getenv(key2, a_toplevel_only, ifnsf, tips);

} // acmacs::settings::v2::Settings::getenv

// ----------------------------------------------------------------------

std::string acmacs::settings::v2::Settings::getenv_to_string(std::string_view key, toplevel_only a_toplevel_only) const
{
    AD_LOG(acmacs::log::settings, "getenv_to_string \"{}\"", key);
    if (const auto& val = environment_.get(environment_.substitute_to_string(key), a_toplevel_only); val.is_string()) {
        const rjson::v3::value* orig = &val;
        for (size_t num_subst = 0; num_subst < 10; ++num_subst) {
            if (orig->empty())
                return std::string{};

            const rjson::v3::value& substituted = std::visit(
                [&orig]<typename Cont>(Cont cont) -> const rjson::v3::value& {
                    if constexpr (std::is_same_v<Cont, const rjson::v3::value*>) {
                        if (cont->is_null())
                            return *orig;
                        else
                            return *cont;
                    }
                    else if constexpr (std::is_same_v<Cont, no_substitution_request>) {
                        return rjson::v3::const_null;
                    }
                    else { // no subst or partial subst
                        if (cont != orig->to<std::string_view>())
                            return rjson::v3::const_empty_string; // --> use substitute_to_string
                        else
                            return rjson::v3::const_null;
                    }
                },
                environment_.substitute(orig->to<std::string_view>()));

            AD_LOG(acmacs::log::settings, "getenv_to_string substituted {} -> {}", *orig, substituted);
            if (substituted.is_null())
                return std::string{orig->to<std::string_view>()};
            else if (&substituted == &rjson::v3::const_empty_string)
                return environment_.substitute_to_string(orig->to<std::string_view>());
            else if (substituted.is_string())
                orig = &substituted;
            else
                throw error(AD_FORMAT("Settings::getenv_to_string: result of substitutions is not a string: {} <- {}", substituted, val));
        }
        throw error(AD_FORMAT("Settings::getenv_to_string: too many substitutions in {}", val));
    }
    else if (val.is_null())
        return std::string{};   // key not found in environment
    else
        throw error(AD_FORMAT("Settings::getenv_to_string: value is not a string: {}", val));

} // acmacs::settings::v2::Settings::getenv_to_string

// ----------------------------------------------------------------------

acmacs::settings::v2::Settings::substitute_result_t acmacs::settings::v2::Settings::substitute(const rjson::v3::value& source) const
{
    AD_LOG(acmacs::log::settings, "substitute in \"{}\"", source);
    AD_LOG_INDENT;
    return source.visit([this, &source]<typename Arg>(const Arg& arg) -> acmacs::settings::v2::Settings::substitute_result_t {
        if constexpr (std::is_same_v<Arg, rjson::v3::detail::string>)
            return environment_.substitute(arg.template to<std::string_view>());
        else
            return &source;
    });

} // acmacs::settings::v2::Settings::substitute

// ----------------------------------------------------------------------

const rjson::v3::value& acmacs::settings::v2::Settings::substitute_to_value(const rjson::v3::value& source) const
{
    return std::visit(
        [&source]<typename Res>(const Res& res) -> const rjson::v3::value& {
            if constexpr (std::is_same_v<Res, const rjson::v3::value*>)
                return *res;
            else
                return source;
        },
        substitute(source));

} // acmacs::settings::v2::Settings::substitute

// ----------------------------------------------------------------------

bool acmacs::settings::v2::Settings::substitute_to_bool(const rjson::v3::value& source) const
{
    return std::visit(
        [&source]<typename Res>(const Res& res) -> bool {
            if constexpr (std::is_same_v<Res, const rjson::v3::value*>)
                return static_cast<bool>(*res);
            else
                throw error{AD_FORMAT("cannot substitute_to_bool in {}", source)};
        },
        substitute(source));

} // acmacs::settings::v2::Settings::substitute_to_bool

// ----------------------------------------------------------------------

double acmacs::settings::v2::Settings::substitute_to_double(const rjson::v3::value& source) const
{
    return std::visit(
        [&source]<typename Res>(const Res& res) -> double {
            if constexpr (std::is_same_v<Res, const rjson::v3::value*>)
                return res->template to<double>();
            else
                throw error{AD_FORMAT("cannot substitute_to_double in {}", source)};
        },
        substitute(source));

} // acmacs::settings::v2::Settings::substitute_to_double

// ----------------------------------------------------------------------

std::string acmacs::settings::v2::Settings::substitute_to_string(const rjson::v3::value& source) const
{
    return std::visit(
        []<typename Res>(const Res& res) -> std::string {
            if constexpr (std::is_same_v<Res, const rjson::v3::value*>)
                return std::string{res->template to<std::string_view>()};
            else if constexpr (std::is_same_v<Res, no_substitution_request>)
                return *res;
            else
                return res;
        },
        substitute(source));

} // acmacs::settings::v2::Settings::substitute_to_string

// ----------------------------------------------------------------------

bool acmacs::settings::v2::Settings::eval_and(const rjson::v3::value& condition) const
{
    if (condition.empty()) {
        fmt::print(stderr, "WARNING: empty and clause evaluates to false\n");
        return false;
    }
    const auto& arr = condition.array();
    const bool result = std::all_of(std::begin(arr), std::end(arr), [this](const rjson::v3::value& sub_condition) { return eval_condition(sub_condition); });
    // fmt::print(stderr, "DEBUG: \"and\" : {} yields {}\n", condition, result);
    return result;

} // acmacs::settings::v2::Settings::eval_and

// ----------------------------------------------------------------------

bool acmacs::settings::v2::Settings::eval_or(const rjson::v3::value& condition) const
{
    if (condition.empty()) {
        fmt::print(stderr, "WARNING: empty or clause evaluates to false\n");
        return false;
    }
    const auto& arr = condition.array();
    return std::any_of(std::begin(arr), std::end(arr), [this](const rjson::v3::value& sub_condition) { return eval_condition(sub_condition); });

} // acmacs::settings::v2::Settings::eval_or

// ----------------------------------------------------------------------

bool acmacs::settings::v2::Settings::eval_equal(const rjson::v3::value& condition) const
{
    if (condition.empty()) {
        fmt::print(stderr, "WARNING: empty equal clause evaluates to false\n");
        return false;
    }
    if (!condition.is_array() || condition.size() < 2) {
        fmt::print(stderr, "WARNING: equal clause condition must be an array with 2 or more elements: {}, evaluates to false\n", condition);
        return false;
    }

    const auto& first = substitute_to_value(condition[0]);
    for (size_t index = 1; index < condition.size(); ++index) {
        if (!(substitute_to_value(condition[index]) == first))
            return false;
    }
    return true;

} // acmacs::settings::v2::Settings::eval_equal

// ----------------------------------------------------------------------

bool acmacs::settings::v2::Settings::eval_empty(const rjson::v3::value& condition, bool true_if_empty) const
{
    try {
        return substitute_to_value(condition).visit([true_if_empty]<typename Arg>(const Arg& arg) -> bool {
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

} // acmacs::settings::v2::Settings::eval_empty

// ----------------------------------------------------------------------
// Load
// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::LoadedDataFiles::load(std::string_view filename)
{
    file_data_.insert(file_data_.begin(), rjson::v3::parse_file(filename));
    filenames_.insert(filenames_.begin(), std::string{filename});

} // acmacs::settings::v2::Settings::LoadedDataFiles::load

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::LoadedDataFiles::load_from_string(std::string_view data)
{
    file_data_.insert(file_data_.begin(), rjson::v3::parse_string(data));
    filenames_.insert(filenames_.begin(), std::string{});

} // acmacs::settings::v2::Settings::LoadedDataFiles::load_from_string

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::LoadedDataFiles::reload(Settings& settings)
{
    using namespace std::string_view_literals;
    if (!filenames_.empty()) {
        for (auto index{filenames_.size()}; index > 0; --index) {
            if (!filenames_[index - 1].empty()) {
                AD_LOG(acmacs::log::settings, "re-loading {}", filenames_[index - 1]);
                file_data_[index - 1] = rjson::v3::parse_file(filenames_[index - 1]);
            }
            if (const auto& val = file_data_[index - 1]["init"sv]; !val.is_null())
                settings.apply(val);
        }
    }

} // acmacs::settings::v2::Settings::LoadedDataFiles::reload

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::load(std::string_view filename)
{
    using namespace std::string_view_literals;
    AD_LOG(acmacs::log::settings, "loading {}", filename);
    loaded_data_.load(filename);
    apply_top("init"sv);

} // acmacs::settings::v2::Settings::load

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::load(const std::vector<std::string_view>& filenames)
{
    for (const auto& filename : filenames)
        load(filename);

} // acmacs::settings::v2::Settings::load

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::load_from_conf(const std::vector<std::string_view>& filenames) // load from ${ACMACSD_ROOT}/share/conf dir
{
    for (const auto& settings_file_name : filenames) {
        if (const auto filename = fmt::format("{}/share/conf/{}", acmacs::acmacsd_root(), settings_file_name); fs::exists(filename))
            load(filename);
        else
            AD_WARNING("cannot load \"{}\": file not found", filename);
    }

} // acmacs::settings::v2::Settings::load_from_conf

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::set_defines(const std::vector<std::string_view>& defines)
{
    using namespace std::string_view_literals;

    for (const auto& def : defines) {
        if (const auto pos = def.find('='); pos != std::string_view::npos) {
            const auto val_s = def.substr(pos + 1);
            if (val_s == "-") { // parsed as -0
                setenv(def.substr(0, pos), rjson::v3::parse_string(fmt::format("\"{}\"", val_s)));
            }
            else {
                try {
                    setenv(def.substr(0, pos), rjson::v3::parse_string(val_s));
                }
                catch (std::exception&) {
                    setenv(def.substr(0, pos), rjson::v3::parse_string(fmt::format("\"{}\"", val_s)));
                }
            }
        }
        else
            setenv(def, "true"sv);
    }

} // acmacs::settings::v2::Settings::set_defines

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::load_from_string(std::string_view data)
{
    loaded_data_.load_from_string(data);
    apply_top("init");

} // acmacs::settings::v2::Settings::load_from_string

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::reload()
{
    loaded_data_.reload(*this);

} // acmacs::settings::v2::Settings::reload

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
