#include <iostream>
#include <cstring>
#include <algorithm>

#include "argc-argv.hh"

// ----------------------------------------------------------------------

#ifdef __clang__
inline const char* std::bad_variant_access::what() const noexcept { return "bad_variant_access"; }
#endif

argc_argv::argc_argv(int argc, const char* const argv[], std::initializer_list<argc_argv::option_setting> options, bool split_single_dash)
    : mProgram{argv[0]}
{
    using opt_iter = decltype(options.begin());
    std::vector<opt_iter> options_used;
    bool no_more_options = false; // support for --, after it all options are considered to be arguments
    for (int arg_no = 1; arg_no < argc; ++arg_no) {
        if (!no_more_options && argv[arg_no][0] == '-' && argv[arg_no][1] != 0) {
            if (argv[arg_no][1] == '-' && argv[arg_no][2] == 0) { // --
                no_more_options = true;
            }
            else if (auto found = std::find_if(options.begin(), options.end(), [arg=argv[arg_no]](const auto& aOption) { return arg == aOption.option_; }); found != options.end()) {
                options_used.push_back(found);
                auto visitor = [&, name=found->option_, this](auto&& arg) -> void {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, bool>)
                        mOptions.emplace_back(name, !arg);
                    else if constexpr (std::is_same_v<T, string>) {
                        if ((arg_no + 1) < argc)
                            mOptions.emplace_back(name, option_default{argv[++arg_no]});
                        else
                            throw option_requires_argument{name};
                    }
                    else if constexpr (std::is_same_v<T, strings>) {
                        if ((arg_no + 1) < argc) {
                            if (const auto found_opt = std::find_if(std::begin(mOptions), std::end(mOptions), [&name](const auto& opt) { return opt.option_ == name; }); found_opt == std::end(mOptions))
                                mOptions.emplace_back(name, option_default{strings{argv[++arg_no]}});
                            else
                                std::get<T>(found_opt->default_).push_back(argv[++arg_no]);
                        }
                        else
                            throw option_requires_argument{name};
                    }
                    else if constexpr (std::is_same_v<T, long>) {
                        if ((arg_no + 1) < argc)
                            mOptions.emplace_back(name, option_default{std::stol(argv[++arg_no])});
                        else
                            throw option_requires_argument{name};
                    // else if constexpr (std::is_same_v<T, unsigned long>)
                    //     mOptions.emplace_back(name, option_default{std::stoul(argv[++arg_no])});
                    }
                    else if constexpr (std::is_same_v<T, double>) {
                        if ((arg_no + 1) < argc)
                            mOptions.emplace_back(name, option_default{std::stod(argv[++arg_no])});
                        else
                            throw option_requires_argument{name};
                    }
                    else
                        static_assert(std::is_same_v<T, bool>, "non-exhaustive visitor in argc_argv::argc_argv!");
                };
                std::visit(visitor, found->default_);
            }
            else if (argv[arg_no][1] != '-' && split_single_dash) {
                for (const char* item = &argv[arg_no][1]; *item; ++item) {
                    const std::string opt{'-', *item};
                    if (auto found_opt = std::find_if(options.begin(), options.end(), [opt](const auto& aOption) { return aOption.option_ == opt; }); found_opt != options.end())
                        mOptions.emplace_back(opt, true);
                    else
                        throw unrecognized_option(opt);
                }
            }
            else
                throw unrecognized_option(argv[arg_no]);
        }
        else {
            mArguments.push_back(argv[arg_no]);
        }
    }

    for (auto opt_p = options.begin(); opt_p != options.end(); ++opt_p) {
        if (std::find(options_used.begin(), options_used.end(), opt_p) == options_used.end()) {
            mOptions.push_back(*opt_p);
        }
    }

} // argc_argv::argc_argv

// ----------------------------------------------------------------------

const argc_argv::option& argc_argv::operator[](std::string aName) const
{
    const auto found = std::find_if(mOptions.begin(), mOptions.end(), [&aName](const auto& opt) { return opt.option_ == aName; });
    if (found != mOptions.end())
        return *found;
    else
        throw option_not_found{aName};

} // argc_argv::operator[]

// ----------------------------------------------------------------------

std::string argc_argv::usage_options(size_t aIndent) const
{
    auto visitor = [](auto&& arg) -> std::string {
                       using T = std::decay_t<decltype(arg)>;
                       std::string result;
                       if constexpr (std::is_same_v<T, bool>)
                                            ;
                       else if constexpr (std::is_same_v<T, string>)
                                                 result = std::string{' ', '"'} + arg + "\"";
                       else if constexpr (std::is_same_v<T, strings>)
                                                 result = " \"(multiple-occurneces-of-switch-hanled)\"";
                       else if constexpr (std::is_same_v<T, long>)
                                                 result = " " + std::to_string(arg);
                       else if constexpr (std::is_same_v<T, double>)
                                                 result = " " + std::to_string(arg);
                       else
                           static_assert(std::is_same_v<T, bool>, "non-exhaustive visitor in argc_argv::argc_argv!");
                       return result;
                   };

    std::string result;
    const std::string indent(aIndent, ' ');
    for (const auto& opt: mOptions) {
        result += indent + opt.option_ + std::visit(visitor, opt.default_) + '\n';
        if (opt.description_)
            result += indent + indent + opt.description_ + '\n';
    }
    return result;

} // argc_argv::usage_options

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

// argc_argv_simple::option::~option()
// {
// } // argc_argv_simple::option::~option

// argc_argv_simple::option::operator bool() const
// {
//     return true;

// } // argc_argv_simple::option::operator bool

// bool argc_argv_simple::option::really_present() const
// {
//     return true;

// } // argc_argv_simple::option::really_present

// argc_argv_simple::option::operator const char*() const
// {
//     if (!mValue)
//         throw option_has_no_value{std::string{mName} + " has no value"};
//     return mValue;

// } // argc_argv_simple::option::operator const char*

// // argc_argv_simple::option::operator std::string() const
// // {
// //     if (!mValue)
// //         throw option_has_no_value{std::string{mName} + " has no value"};
// //     return mValue;

// // } // argc_argv_simple::option::operator std::string

// argc_argv_simple::option::operator double() const
// {
//     if (!mValue)
//         throw option_has_no_value{std::string{mName} + " has no value"};
//     try {
//         return std::stod(mValue);
//     }
//     catch (std::exception&) {
//         throw option_value_conversion_failed{std::string{"cannot convert value of "} + mName + " to double: " + mValue};
//     }

// } // argc_argv_simple::option::operator double

// argc_argv_simple::option::operator int() const
// {
//     if (!mValue)
//         throw option_has_no_value{std::string{mName} + " has no value"};
//     try {
//         return std::stoi(mValue);
//     }
//     catch (std::exception&) {
//         throw option_value_conversion_failed{std::string{"cannot convert value of "} + mName + " to int: " + mValue};
//     }

// } // argc_argv_simple::option::operator int

// // ----------------------------------------------------------------------

// class no_option : public argc_argv_simple::option
// {
//  public:
//     inline no_option(std::string name) : argc_argv_simple::option(name, nullptr) {}
//     operator bool() const override { return false; }
//     bool really_present() const override { return false; }
//     operator const char*() const override { throw argc_argv_simple::option_not_found{std::string{"option "} + name() + " not found in the command line"}; }
//     // operator std::string() const override { throw argc_argv_simple::option_not_found{std::string{"option "} + name() + " not found in the command line"}; }
//     operator double() const override { throw argc_argv_simple::option_not_found{std::string{"option "} + name() + " not found in the command line"}; }
//     operator int() const override { throw argc_argv_simple::option_not_found{std::string{"option "} + name() + " not found in the command line"}; }

// }; // class no_option

// // ----------------------------------------------------------------------

// class option_with_default : public argc_argv_simple::option
// {
//  public:
//     inline option_with_default(std::string name, std::string value) : argc_argv_simple::option(name, nullptr), mSvalue(value) { set_value(mSvalue.data()); }
//     template <typename T> inline option_with_default(std::string name, T value) : argc_argv_simple::option(name, nullptr), mSvalue(std::to_string(value)) { set_value(mSvalue.data()); }
//     bool really_present() const override { return false; }

//  private:
//     const std::string mSvalue;

// }; // class no_option

// // ----------------------------------------------------------------------

// argc_argv_simple::argc_argv_simple(int argc, const char* const argv[], std::initializer_list<const char*> options_with_value, bool split_single_dash)
//     : mOptionsWithValue{options_with_value}, mProgram{argv[0]}
// {
//     for (int arg_no = 1; arg_no < argc; ++arg_no) {
//         if (argv[arg_no][0] == '-') {
//             if (std::find_if(mOptionsWithValue.begin(), mOptionsWithValue.end(), [arg=argv[arg_no]](const char* aOption) { return !std::strcmp(arg, aOption); }) != mOptionsWithValue.end()) {
//                 if ((arg_no + 1) >= argc)
//                     throw option_has_no_value{std::string{argv[arg_no]} + " requires a value, but it is not in the command line"};
//                 mOptions.emplace_back(new option(argv[arg_no], argv[arg_no + 1]));
//                 // std::cout << "Add option with value: " << argv[arg_no] << ' ' << argv[arg_no + 1] << '\n';
//                 ++arg_no;
//             }
//             else if (argv[arg_no][1] != '-' && split_single_dash) {
//                 for (const char* item = &argv[arg_no][1]; *item; ++item) {
//                     mOptions.emplace_back(new option({'-', *item}));
//                 }
//             }
//             else {
//                 mOptions.emplace_back(new option(argv[arg_no]));
//                 // std::cout << "Add option: " << argv[arg_no] << '\n';
//             }
//         }
//         else {
//             mArguments.push_back(argv[arg_no]);
//         }
//     }

// } // argc_argv_simple::argc_argv_simple

// // ----------------------------------------------------------------------

// const argc_argv_simple::option& argc_argv_simple::operator[](std::string aName) const
// {
//     const auto found = std::find_if(mOptions.begin(), mOptions.end(), [&aName](const auto& opt) { return opt->name() == aName; });
//     if (found != mOptions.end())
//         return *found->get();
//     else
//         return *mOptions.emplace_back(new no_option(aName));

// } // argc_argv_simple::operator[]

// // ----------------------------------------------------------------------

// const argc_argv_simple::option& argc_argv_simple::get(std::string aName) const
// {
//     const auto found = std::find_if(mOptions.begin(), mOptions.end(), [&aName](const auto& opt) { return opt->name() == aName; });
//     if (found != mOptions.end()) {
//         if ((*found)->really_present()) // option was really present or default
//             return *found->get();
//         mOptions.erase(found);
//     }
//     throw option_not_found{""};

// } // argc_argv_simple::get

// // ----------------------------------------------------------------------

// const argc_argv_simple::option& argc_argv_simple::get(std::string aName, std::string aDefault) const
// {
//     try {
//         return get(aName);
//     }
//     catch (option_not_found&) {
//         return *mOptions.emplace_back(new option_with_default(aName, aDefault));
//     }

// } // argc_argv_simple::get

// // ----------------------------------------------------------------------

// const argc_argv_simple::option& argc_argv_simple::get(std::string aName, double aDefault) const
// {
//     try {
//         return get(aName);
//     }
//     catch (option_not_found&) {
//         return *mOptions.emplace_back(new option_with_default(aName, aDefault));
//     }

// } // argc_argv_simple::get

// // ----------------------------------------------------------------------

// const argc_argv_simple::option& argc_argv_simple::get(std::string aName, int aDefault) const
// {
//     try {
//         return get(aName);
//     }
//     catch (option_not_found&) {
//         return *mOptions.emplace_back(new option_with_default(aName, aDefault));
//     }

// } // argc_argv_simple::get

// // ----------------------------------------------------------------------

// const char* argc_argv_simple::operator[](size_t aIndex) const
// {
//     if (aIndex >= mArguments.size())
//         throw argument_not_found{"invalid argument index: " + std::to_string(aIndex) + ", total arguments in the command line: " + std::to_string(mArguments.size())};
//     return mArguments[aIndex];

// } // argc_argv_simple::operator[]

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
