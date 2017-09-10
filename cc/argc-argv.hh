#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <initializer_list>
#include <memory>

// ----------------------------------------------------------------------

class argc_argv
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
          // virtual operator std::string() const;
        virtual operator double() const;
        virtual operator int() const;

     protected:
        inline option(std::string name, const char* value = nullptr) : mName{name}, mValue{value} {}
        friend class argc_argv;

        inline std::string name() const { return mName; }
        inline void set_value(const char* aValue) { mValue = aValue; }
        virtual bool really_present() const;

     private:
        std::string mName;
        const char* mValue;

    }; // class option

      // options_with_value: list of option names that have values, e.g. {"-o", "--output"}
      // may throw option_has_no_value if last value in argv is option from options_with_value
    argc_argv(int argc, const char* const argv[], std::initializer_list<const char*> options_with_value, bool split_single_dash = true);

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

}; // class argc_argv

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
