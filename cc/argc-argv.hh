#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <initializer_list>
#include <memory>
#include <variant>
#include <typeinfo>

#include "acmacs-base/string.hh"

// ----------------------------------------------------------------------

class argc_argv
{
 public:
    class unrecognized_option : public std::runtime_error { public: unrecognized_option(std::string opt) : std::runtime_error("unrecognized command line option: " + opt) {} };
    class option_not_found : public std::runtime_error { public: option_not_found(std::string opt) : std::runtime_error("internal: option_not_found: " + opt) {} };
    class option_requires_argument : public std::runtime_error { public: option_requires_argument(std::string opt) : std::runtime_error("command line option " + opt + " requires argument") {} };
    class argument_not_found : public std::runtime_error { public: using std::runtime_error::runtime_error; };
    class option_value_conversion_failed : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    using string = const char*;
    using strings = std::vector<string>;

    using option_default = std::variant<bool, string, strings, long, double>;

    struct option_setting
    {
        option_setting(const char* aOption, option_default&& aDefault) : option_(aOption), default_(std::move(aDefault)) {}
        option_setting(std::string aOption, option_default&& aDefault) : option_(aOption), default_(std::move(aDefault)) {}
        option_setting(std::string aOption, const option_default& aDefault) : option_(aOption), default_(aDefault) {}
        option_setting(const char* aOption, option_default&& aDefault, const char* aDescription) : option_(aOption), default_(std::move(aDefault)), description_(aDescription) {}
        option_setting(const char* aOption, int aDefault, const char* aDescription = nullptr) : option_(aOption), default_(static_cast<long>(aDefault)), description_(aDescription) {}
        option_setting(const char* aOption, double aDefault, const char* aDescription = nullptr) : option_(aOption), default_(aDefault), description_(aDescription) {}
        option_setting(const char* aOption, bool aDefault, const char* aDescription = nullptr) : option_(aOption), default_(aDefault), description_(aDescription) {}
        option_setting(const char* aOption, const char* aDefault, const char* aDescription = nullptr) : option_(aOption), default_(string{aDefault}), description_(aDescription) {}

        std::string option_;
        option_default default_;
        const char* description_ = nullptr;

    }; // struct option_setting

    class option : public option_setting
    {
     private:
        bool present_ = false;

        template <typename T> T get() const
            {
                using namespace std::string_literals;
                try {
                    return std::get<T>(default_);
                }
                catch (std::bad_variant_access&) {
                    throw option_value_conversion_failed{"cannot convert value of "s + option_ + " to " + typeid(T).name()};
                }
            }

        bool get_bool() const
            {
                using namespace std::string_literals;
                if (auto* b = std::get_if<bool>(&default_); b)
                    return *b;
                else if (auto* s1 = std::get_if<string>(&default_); s1)
                    return *s1 != nullptr && **s1 != 0;
                else if (auto* s2 = std::get_if<strings>(&default_); s2)
                    return !s2->empty();
                else
                    throw option_value_conversion_failed{"cannot convert value of "s + option_ + " to bool"};
            }

     public:
        option(std::string aOption, option_default&& aDefault) : option_setting(aOption, std::move(aDefault)), present_{true} {}
        option(const option_setting& src) : option_setting(src) {}

        constexpr bool present() const { return present_; }

        operator bool() const { return get_bool(); }
        explicit operator std::string() const { return get<string>(); }
        operator std::string_view() const { return get<string>(); }
        operator strings() const { return get<strings>(); }
        operator double() const { return get<double>(); }
        operator long() const { return get<long>(); }
        operator int() const { return static_cast<int>(get<long>()); }
        operator unsigned int() const { return static_cast<unsigned int>(get<long>()); }
        operator unsigned long() const { return static_cast<unsigned long>(get<long>()); }

        std::string str() const { return get<string>(); }
        std::string_view str_view() const { return get<string>(); }

        std::string as_string() const;

    }; // class option

    argc_argv(int argc, const char* const argv[], std::initializer_list<option_setting> options, bool split_single_dash = true);

    const option& operator[](std::string aName) const;

      // returns number of arguments in the command line that are neither options nor option values
    size_t number_of_arguments() const { return mArguments.size(); }

      // returns argument (neither option nor option value) by index, throws argument_not_found if aIndex >= number_of_arguments()
    std::string_view operator[](size_t aIndex) const
        {
            if (aIndex >= mArguments.size())
                throw argument_not_found{"invalid argument index: " + std::to_string(aIndex) + ", total arguments in the command line: " + std::to_string(mArguments.size())};
            return mArguments[aIndex];
        }

      // returns argv[0]
    const char* program() const { return mProgram; }

    std::string usage_options(size_t aIndent = 2) const;

 private:
    const char* mProgram;
    std::vector<const char*> mArguments;
    mutable std::vector<option> mOptions;

    // const option& get(std::string aName) const;

}; // class argc_argv

inline bool operator == (const argc_argv::option& aOpt, const char* aStr) { return static_cast<std::string_view>(aOpt) == aStr; }
inline bool operator == (const char* aStr, const argc_argv::option& aOpt) { return static_cast<std::string_view>(aOpt) == aStr; }
inline bool operator != (const argc_argv::option& aOpt, const char* aStr) { return !operator==(aOpt, aStr); }
inline bool operator != (const char* aStr, const argc_argv::option& aOpt) { return !operator==(aStr, aOpt); }

namespace acmacs
{
    inline std::string to_string(const argc_argv::option& opt) { return opt.as_string(); }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
