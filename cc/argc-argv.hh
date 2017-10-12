#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <initializer_list>
#include <memory>
#include <variant>
#include <typeinfo>

// ----------------------------------------------------------------------

class argc_argv
{
 public:
    class unrecognized_option : public std::runtime_error { public: using std::runtime_error::runtime_error; };
    class option_not_found : public std::runtime_error { public: using std::runtime_error::runtime_error; };
    class option_requires_argument : public std::runtime_error { public: using std::runtime_error::runtime_error; };
    class argument_not_found : public std::runtime_error { public: using std::runtime_error::runtime_error; };
    class option_value_conversion_failed : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    // using mandatory = std::variant<std::string, int, size_t, double, bool>;
    using option_default = std::variant<bool, const char*, long, double>;

    class option : public std::pair<std::string, option_default>
    {
     private:
        template <typename T> inline T get() const
            {
                try {
                    return std::get<T>(second);
                }
                catch (std::bad_variant_access&) {
                    throw option_value_conversion_failed{std::string{"cannot convert value of "} + first + " to " + typeid(T).name()};
                }
            }

        inline bool get_bool() const
            {
                if (auto* b = std::get_if<bool>(&second); b)
                    return *b;
                else if (auto* const * s = std::get_if<const char*>(&second); s)
                    return *s != nullptr && **s != 0;
                else
                    throw option_value_conversion_failed{std::string{"cannot convert value of "} + first + " to bool"};
            }

     public:
        using std::pair<std::string, option_default>::pair;

        inline operator bool() const { return get_bool(); }
        inline operator std::string() const { return get<const char*>(); }
        inline operator double() const { return get<double>(); }
        inline operator long() const { return get<long>(); }
        inline operator int() const { return static_cast<int>(get<long>()); }
        inline operator unsigned int() const { return static_cast<unsigned int>(get<long>()); }
        inline operator unsigned long() const { return static_cast<unsigned long>(get<long>()); }

    }; // class option

    argc_argv(int argc, const char* const argv[], std::initializer_list<std::pair<const char*, option_default>> options, bool split_single_dash = true);

    const option& operator[](std::string aName) const;

      // returns number of arguments in the command line that are neither options nor option values
    inline size_t number_of_arguments() const { return mArguments.size(); }

      // returns argument (neither option nor option value) by index, throws argument_not_found if aIndex >= number_of_arguments()
    inline const char* operator[](size_t aIndex) const
        {
            if (aIndex >= mArguments.size())
                throw argument_not_found{"invalid argument index: " + std::to_string(aIndex) + ", total arguments in the command line: " + std::to_string(mArguments.size())};
            return mArguments[aIndex];
        }

      // returns argv[0]
    inline const char* program() const { return mProgram; }

    std::string usage_options(size_t aIndent = 2) const;

 private:
    const char* mProgram;
    std::vector<const char*> mArguments;
    mutable std::vector<option> mOptions;

    // const option& get(std::string aName) const;

}; // class argc_argv

inline bool operator == (const argc_argv::option& aOpt, const char* aStr) { return static_cast<std::string>(aOpt) == aStr; }
inline bool operator == (const char* aStr, const argc_argv::option& aOpt) { return static_cast<std::string>(aOpt) == aStr; }
inline bool operator != (const argc_argv::option& aOpt, const char* aStr) { return !operator==(aOpt, aStr); }
inline bool operator != (const char* aStr, const argc_argv::option& aOpt) { return !operator==(aStr, aOpt); }

// ----------------------------------------------------------------------

class argc_argv_simple
{
 public:
    class option_not_found : public std::runtime_error { public: using std::runtime_error::runtime_error; };
    class argument_not_found : public std::runtime_error { public: using std::runtime_error::runtime_error; };
    class option_has_no_value : public std::runtime_error { public: using std::runtime_error::runtime_error; };
    class option_value_conversion_failed : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    class option
    {
     public:
        virtual ~option();

          // if option was present in the command line
        virtual operator bool() const;

          // argument extraction, throws option_not_found, option_has_no_value, option_value_conversion_failed
        virtual operator const char*() const;
        inline operator std::string() const { return operator const char*(); }
        virtual operator double() const;
        virtual operator int() const;

     protected:
        inline option(std::string name, const char* value = nullptr) : mName{name}, mValue{value} {}
        friend class argc_argv_simple;

        inline std::string name() const { return mName; }
        inline void set_value(const char* aValue) { mValue = aValue; }
        virtual bool really_present() const;

     private:
        std::string mName;
        const char* mValue;

    }; // class option

      // options_with_value: list of option names that have values, e.g. {"-o", "--output"}
      // may throw option_has_no_value if last value in argv is option from options_with_value
    argc_argv_simple(int argc, const char* const argv[], std::initializer_list<const char*> options_with_value, bool split_single_dash = true);

      // returns option by name, if option was not in argv, returns ref to special option object
    const option& operator[](std::string aName) const;

      // returns option by name, if option was not in argv, returns ref to an option object having aDefault as a value
    const option& get(std::string aName, std::string aDefault) const;
    const option& get(std::string aName, double aDefault) const;
    const option& get(std::string aName, int aDefault) const;

      // returns number of arguments in the command line that are neither options nor option values
    inline size_t number_of_arguments() const { return mArguments.size(); }

      // returns argument (neither option nor option value) by index, throws argument_not_found if aIndex >= number_of_arguments()
    const char* operator[](size_t aIndex) const;

      // returns argv[0]
    inline const char* program() const { return mProgram; }

 private:
    const std::vector<const char*> mOptionsWithValue;
    const char* mProgram;
    std::vector<const char*> mArguments;
    mutable std::vector<std::unique_ptr<option>> mOptions;

    const option& get(std::string aName) const;

}; // class argc_argv_simple

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
