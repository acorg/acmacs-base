#include <iostream>
#include <cstring>

#include "argc-argv.hh"

// ----------------------------------------------------------------------

argc_argv::option::~option()
{
} // argc_argv::option::~option

argc_argv::option::operator bool() const
{
    return true;

} // argc_argv::option::operator bool

bool argc_argv::option::really_present() const
{
    return true;

} // argc_argv::option::really_present

argc_argv::option::operator const char*() const
{
    if (!mValue)
        throw option_has_no_value{std::string{mName} + " has no value"};
    return mValue;

} // argc_argv::option::operator const char*

// argc_argv::option::operator std::string() const
// {
//     if (!mValue)
//         throw option_has_no_value{std::string{mName} + " has no value"};
//     return mValue;

// } // argc_argv::option::operator std::string

argc_argv::option::operator double() const
{
    if (!mValue)
        throw option_has_no_value{std::string{mName} + " has no value"};
    try {
        return std::stod(mValue);
    }
    catch (std::exception&) {
        throw option_value_conversion_failed{std::string{"cannot convert value of "} + mName + " to double: " + mValue};
    }

} // argc_argv::option::operator double

argc_argv::option::operator int() const
{
    if (!mValue)
        throw option_has_no_value{std::string{mName} + " has no value"};
    try {
        return std::stoi(mValue);
    }
    catch (std::exception&) {
        throw option_value_conversion_failed{std::string{"cannot convert value of "} + mName + " to int: " + mValue};
    }

} // argc_argv::option::operator int

// ----------------------------------------------------------------------

class no_option : public argc_argv::option
{
 public:
    inline no_option(std::string name) : argc_argv::option(name, nullptr) {}
    operator bool() const override { return false; }
    bool really_present() const override { return false; }
    operator const char*() const override { throw argc_argv::option_not_found{std::string{"option "} + name() + " not found in the command line"}; }
    // operator std::string() const override { throw argc_argv::option_not_found{std::string{"option "} + name() + " not found in the command line"}; }
    operator double() const override { throw argc_argv::option_not_found{std::string{"option "} + name() + " not found in the command line"}; }
    operator int() const override { throw argc_argv::option_not_found{std::string{"option "} + name() + " not found in the command line"}; }

}; // class no_option

// ----------------------------------------------------------------------

class option_with_default : public argc_argv::option
{
 public:
    inline option_with_default(std::string name, std::string value) : argc_argv::option(name, nullptr), mSvalue(value) { set_value(mSvalue.data()); }
    template <typename T> inline option_with_default(std::string name, T value) : argc_argv::option(name, nullptr), mSvalue(std::to_string(value)) { set_value(mSvalue.data()); }
    bool really_present() const override { return false; }

 private:
    const std::string mSvalue;

}; // class no_option

// ----------------------------------------------------------------------

argc_argv::argc_argv(int argc, const char* const argv[], std::initializer_list<const char*> options_with_value, bool split_single_dash)
    : mOptionsWithValue{options_with_value}, mProgram{argv[0]}
{
    for (int arg_no = 1; arg_no < argc; ++arg_no) {
        if (argv[arg_no][0] == '-') {
            if (std::find_if(mOptionsWithValue.begin(), mOptionsWithValue.end(), [arg=argv[arg_no]](const char* aOption) { return !std::strcmp(arg, aOption); }) != mOptionsWithValue.end()) {
                if ((arg_no + 1) >= argc)
                    throw option_has_no_value{std::string{argv[arg_no]} + " requires a value, but it is not in the command line"};
                mOptions.emplace_back(new option(argv[arg_no], argv[arg_no + 1]));
                // std::cout << "Add option with value: " << argv[arg_no] << ' ' << argv[arg_no + 1] << '\n';
                ++arg_no;
            }
            else if (argv[arg_no][1] != '-' && split_single_dash) {
                for (const char* item = &argv[arg_no][1]; *item; ++item) {
                    mOptions.emplace_back(new option({'-', *item}));
                }
            }
            else {
                mOptions.emplace_back(new option(argv[arg_no]));
                // std::cout << "Add option: " << argv[arg_no] << '\n';
            }
        }
        else {
            mArguments.push_back(argv[arg_no]);
        }
    }

} // argc_argv::argc_argv

// ----------------------------------------------------------------------

const argc_argv::option& argc_argv::operator[](std::string aName) const
{
    const auto found = std::find_if(mOptions.begin(), mOptions.end(), [&aName](const auto& opt) { return opt->name() == aName; });
    if (found != mOptions.end())
        return *found->get();
    else
        return *mOptions.emplace_back(new no_option(aName));

} // argc_argv::operator[]

// ----------------------------------------------------------------------

const argc_argv::option& argc_argv::get(std::string aName) const
{
    const auto found = std::find_if(mOptions.begin(), mOptions.end(), [&aName](const auto& opt) { return opt->name() == aName; });
    if (found != mOptions.end()) {
        if ((*found)->really_present()) // option was really present or default
            return *found->get();
        mOptions.erase(found);
    }
    throw option_not_found{""};

} // argc_argv::get

// ----------------------------------------------------------------------

const argc_argv::option& argc_argv::get(std::string aName, std::string aDefault) const
{
    try {
        return get(aName);
    }
    catch (option_not_found&) {
        return *mOptions.emplace_back(new option_with_default(aName, aDefault));
    }

} // argc_argv::get

// ----------------------------------------------------------------------

const argc_argv::option& argc_argv::get(std::string aName, double aDefault) const
{
    try {
        return get(aName);
    }
    catch (option_not_found&) {
        return *mOptions.emplace_back(new option_with_default(aName, aDefault));
    }

} // argc_argv::get

// ----------------------------------------------------------------------

const argc_argv::option& argc_argv::get(std::string aName, int aDefault) const
{
    try {
        return get(aName);
    }
    catch (option_not_found&) {
        return *mOptions.emplace_back(new option_with_default(aName, aDefault));
    }

} // argc_argv::get

// ----------------------------------------------------------------------

const char* argc_argv::operator[](size_t aIndex) const
{
    if (aIndex >= mArguments.size())
        throw argument_not_found{"invalid argument index: " + std::to_string(aIndex) + ", total arguments in the command line: " + std::to_string(mArguments.size())};
    return mArguments[aIndex];

} // argc_argv::operator[]

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
