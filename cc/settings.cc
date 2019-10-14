#include <regex>

#include "acmacs-base/settings.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-base/string.hh"

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

inline rjson::value acmacs::settings::v2::Settings::Environment::substitute(const rjson::value& source) const
{
    return std::visit([this,&source]<typename T>(T&& arg) -> rjson::value {
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
            return substitute(std::string_view{arg});
        else
            return source;
    }, source.val_());

} // acmacs::settings::v2::Settings::Environment::substitute

// ----------------------------------------------------------------------

rjson::value acmacs::settings::v2::Settings::Environment::substitute(std::string_view source) const
{
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

    static std::regex re{"\\{([^\\}]+)\\}"};

#pragma GCC diagnostic pop

    if (std::cmatch m1; std::regex_search(std::begin(source), std::end(source), m1, re)) {
        if (const auto& found1 = get(m1.str(1)); found1.is_const_null()) {
            return found1;
            // throw error(fmt::format("cannot find substitution for \"{}\" in environment, source: \"{}\"", m1.str(1), source));
        }
        else if (m1.position(0) == 0 && static_cast<size_t>(m1.length(0)) == source.size()) { // entire source matched
            return found1;
        }
        else {
            const auto replace = [](const auto& src, size_t prefix, const rjson::value& infix, size_t suffix) {
                return fmt::format("{}{}{}", src.substr(0, prefix), infix.is_string() ? infix.to<std::string>() : rjson::to_string(infix), src.substr(suffix));
            };

            std::string result = replace(source, static_cast<size_t>(m1.position(0)), found1, static_cast<size_t>(m1.position(0) + m1.length(0)));
            std::smatch m2;
            while (std::regex_search(result, m2, re)) {
                if (const auto& found2 = get(m2.str(1)); found2.is_const_null())
                    throw error(fmt::format("cannot find substitution for \"{}\" in environment, source: \"{}\"", m1.str(1), source));
                else
                    result = replace(result, static_cast<size_t>(m2.position(0)), found2, static_cast<size_t>(m2.position(0) + m2.length(0)));
            }
            return result;
        }
    }
    else
        return std::string{source};

} // acmacs::settings::v2::Settings::Environment::substitute

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::load(std::string_view filename)
{
    data_.push_back(rjson::parse_file(filename, rjson::remove_comments::no));
    apply_top("init");

} // acmacs::settings::v2::Settings::load

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::setenv_from_string(std::string_view key, std::string_view value)
{
    if (value == "true") {
        setenv(key, true);
    }
    else if (value == "false") {
        setenv(key, true);
    }
    else if (value.empty() || value == "null") {
        setenv(key, rjson::ConstNull);
    }
    else if (value.front() == '"') {
        if (value.back() == '"')
            setenv(key, value.substr(1, value.size() - 2));
        else
            setenv(key, value);
    }
    else {
        size_t processed = 0;
        try {
            if (const auto val = ::string::from_chars<int>(value, processed); processed == value.size())
                setenv(key, val);
        }
        catch (std::exception&) {
        }
        if (processed != value.size()) {
            try {
                if (const auto val = ::string::from_chars<double>(value, processed); processed == value.size())
                    setenv(key, val);
            }
            catch (std::exception&) {
            }
        }
        if (processed != value.size())
            setenv(key, std::string{value});
    }

} // acmacs::settings::v2::Settings::setenv_from_string

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::apply(std::string_view name) const
{
    // fmt::print(stderr, "DEBUG: Settings::apply \"{}\"\n", name);
    if (name.empty())
        throw error("cannot apply command with an empty name");
    if (name.front() != '?') { // not commented out
        const auto substituted_name = environment_.substitute(name).to<std::string>();
        if (const auto& val1 = environment_.get(substituted_name); !val1.is_const_null()) {
            apply(val1);
        }
        else if (const auto& val2 = get(substituted_name); !val2.is_const_null())
            apply(val2);
        else if (!apply_built_in(substituted_name))
            throw error(fmt::format("settings entry not found: \"{}\" (not substituted: \"{}\")", substituted_name, name));
    }

} // acmacs::settings::v2::Settings::apply

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::apply_top(std::string_view name) const
{
    if (name.empty())
        throw error("cannot apply command with an empty name");
    if (name.front() != '?') { // not commented out
        const auto substituted_name = environment_.substitute(name).to<std::string>();
        if (const auto& val = data_.back().get(substituted_name); !val.is_const_null())
            apply(val);
    }

} // acmacs::settings::v2::Settings::apply_top

// ----------------------------------------------------------------------

bool acmacs::settings::v2::Settings::apply_built_in(std::string_view name) const
{
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
        throw error{fmt::format("cannot apply \"{}\": {}", name, err)};
    }

} // acmacs::settings::v2::Settings::apply_built_in

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::apply(const rjson::value& entry) const
{
    try {
        // fmt::print(stderr, "INFO: settings::apply: {}\n", rjson::to_string(entry));
        rjson::for_each(entry, [this](const rjson::value& sub_entry) {
            std::visit(
                [this]<typename T>(T&& sub_entry_val) {
                    // fmt::print(stderr, "DEBUG: apply {}\n", sub_entry_val);
                    if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
                        this->apply(std::string_view{sub_entry_val});
                    else if constexpr (std::is_same_v<std::decay_t<T>, rjson::object>)
                        this->push_and_apply(sub_entry_val);
                    else
                        throw error(fmt::format("cannot apply: {}\n", rjson::to_string(sub_entry_val)));
                },
                sub_entry.val_());
        });
    }
    catch (rjson::value_type_mismatch& err) {
        throw error(fmt::format("cannot apply: {} : {}\n", err, rjson::to_string(entry)));
    }

} // acmacs::settings::v2::Settings::apply

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::push_and_apply(const rjson::object& entry) const
{
    fmt::print(stderr, "INFO: settings::push_and_apply: {}\n", rjson::to_string(entry));
    try {
        if (const auto& command_v = entry.get("N"); !command_v.is_const_null()) {
            const auto command{command_v.to<std::string_view>()};
            // fmt::print(stderr, "DEBUG: Settings::push_and_apply command {} -> {}\n", command_v, command);
            Subenvironment sub_env(environment_, command != "set");
            entry.for_each([this](const std::string& key, const rjson::value& val) {
                if (key != "N") {
                    // fmt::print(stderr, "DEBUG: Settings env add \"{}\": {}\n", key, val);
                    environment_.add(key, val);
                }
            });
            if (command != "set")
                apply(command);
            else if (warn_if_set_used_)
                fmt::print(stderr, "WARNING: \"set\" command has no effect (used inside \"if\"?): {}\n", entry);
        }
        else if (const auto& commented_command_v = entry.get("?N"); !commented_command_v.is_const_null()) {
            // command is commented out
        }
        else
            throw error(fmt::format("cannot apply: {}\n", rjson::to_string(entry)));
    }
    catch (rjson::value_type_mismatch& err) {
        throw error(fmt::format("cannot apply: {} : {}\n", err, rjson::to_string(entry)));
    }


} // acmacs::settings::v2::Settings::push_and_apply

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::apply_if() const
{
    raii_true warn_if_set_used{warn_if_set_used_};
    if (const auto& condition_clause = getenv("condition"); eval_condition(condition_clause)) {
        // fmt::print(stderr, "DEBUG: apply then: {}\n", getenv("then"));
        if (const auto& then_clause = getenv("then"); !then_clause.is_null()) {
            if (!then_clause.is_array())
                throw error{"\"then\" clause must be array"};
            apply(then_clause);
        }
    }
    else {
        if (const auto& else_clause = getenv("else"); !else_clause.is_null()) {
            if (!else_clause.is_array())
                throw error{"\"else\" clause must be array"};
            apply(else_clause);
        }
    }

} // acmacs::settings::v2::Settings::apply_if

// ----------------------------------------------------------------------

bool acmacs::settings::v2::Settings::eval_condition(const rjson::value& condition) const
{
    try {
        return std::visit(
            [this]<typename T>(T && arg)->bool {
                if constexpr (std::is_same_v<std::decay_t<T>, rjson::null> || std::is_same_v<std::decay_t<T>, rjson::const_null>)
                    return false;
                else if constexpr (std::is_same_v<std::decay_t<T>, rjson::number>)
                    return !float_zero(rjson::to_double(arg));
                else if constexpr (std::is_same_v<std::decay_t<T>, bool>)
                    return arg;
                else if constexpr (std::is_same_v<std::decay_t<T>, rjson::object>) {
                    if (arg.size() != 1)
                        throw error{"object must have exactly one key"};
                    if (const auto& and_clause = arg.get("and"); !and_clause.is_null())
                        return eval_and(and_clause);
                    else if (const auto& or_clause = arg.get("or"); !or_clause.is_null())
                        return eval_or(or_clause);
                    else if (const auto& not_clause = arg.get("not"); !not_clause.is_null())
                        return eval_not(not_clause);
                    else
                        throw error{"unrecognized clause"};
                }
                else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
                    if (const rjson::value substituted = environment_.substitute(std::string_view{arg}); substituted.is_string() && substituted.to<std::string_view>() == arg)
                        throw error{"unsupported value type"};
                    else
                        return eval_condition(substituted);
                }
                else if constexpr (std::is_same_v<std::decay_t<T>, rjson::array>)
                    throw error{"unsupported value type"};
            },
            condition.val_());
    }
    catch (std::exception& err) {
        throw error(fmt::format("cannot eval condition: {} -- condition: {}\n", err, condition));
    }

} // acmacs::settings::v2::Settings::eval_condition

// ----------------------------------------------------------------------

bool acmacs::settings::v2::Settings::eval_and(const rjson::value& condition) const
{
    if (condition.empty()) {
        fmt::print(stderr, "WARNING: empty and clause evaluates to false\n");
        return false;
    }
    bool result = true;
    rjson::for_each(condition, [this, &result](const rjson::value& sub_condition) { result &= eval_condition(sub_condition); });
    // fmt::print(stderr, "DEBUG: \"and\" : {} yields {}\n", condition, result);
    return result;

} // acmacs::settings::v2::Settings::eval_and

// ----------------------------------------------------------------------

bool acmacs::settings::v2::Settings::eval_or(const rjson::value& condition) const
{
    if (condition.empty()) {
        fmt::print(stderr, "WARNING: empty and clause evaluates to false\n");
        return false;
    }
    bool result = false;
    rjson::for_each(condition, [this, &result](const rjson::value& sub_condition) { result |= eval_condition(sub_condition); });
    return result;

} // acmacs::settings::v2::Settings::eval_or

// ----------------------------------------------------------------------

bool acmacs::settings::v2::Settings::eval_not(const rjson::value& condition) const
{
    return !eval_condition(condition);

} // acmacs::settings::v2::Settings::eval_not

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
            fmt::print("    {} \"{}\": {}\n", level, entry.first, rjson::to_string(substitute(entry.second)));
    }

} // acmacs::settings::v2::Settings::Environment::print

// ----------------------------------------------------------------------

void acmacs::settings::v2::Settings::Environment::print_key_value() const
{
    fmt::print("{}: {}\n", rjson::to_string(get("key")), rjson::to_string(substitute(get("value"))));

} // acmacs::settings::v2::Settings::Environment::print_value

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
