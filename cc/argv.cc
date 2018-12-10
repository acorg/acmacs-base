#include <iostream>
#include <algorithm>
#include <cstdlib>

#include "acmacs-base/argv.hh"

// ----------------------------------------------------------------------

std::ostream& acmacs::argv::v2::operator<<(std::ostream& out, const argv& args)
{
    out << "argv0: " << args.prog_name_ << '\n';
    for (const auto* opt : args.options_)
        out << *opt << '\n';
    for (const auto& arg : args.args_)
        out << '"' << arg << "\"\n";
    return out;

} // acmacs::argv::v2::operator<<

// ----------------------------------------------------------------------

void acmacs::argv::v2::detail::option_base::show_names(std::ostream& out) const
{
    if (short_name_) {
        out << '-' << short_name_;
        if (!long_name_.empty())
            out << ',';
    }
    if (!long_name_.empty())
        out << "--" << long_name_;

} // acmacs::argv::v2::detail::option_base::show_names

// ----------------------------------------------------------------------

acmacs::argv::v2::argv::argv(int argc, const char* const argv[])
    : prog_name_(argv[0])
{
    const std::vector command_line(argv + 1, argv + argc);
    for (auto arg = command_line.begin(); arg != command_line.end(); ++arg)
        use(arg);
    if (!errors_.empty()) {
        for (const auto& err: errors_)
            std::cerr << "ERROR: " << err << '\n';
    }
    if (!errors_.empty() || show_help_) {
        std::cerr << "SHOW HELP\n";
        std::exit(errors_.empty() ? 0 : 1);
    }

} // acmacs::argv::v2::argv::argv

// ----------------------------------------------------------------------

void acmacs::argv::v2::argv::use(detail::cmd_line_iter& arg)
{
    if ((*arg)[0] == '-') {
        if ((*arg)[1] == '-') {
            if ((*arg)[2])
                ;               // long opt
            else
                args_.push_back(*arg); // just two dashes
        }
        else if ((*arg)[1]) {   // short opts
            for (const char* short_name = *arg + 1; *short_name; ++short_name) {
                if (auto* opt = find(*short_name); opt)
                    opt->add(arg);
                else
                    errors_.push_back(std::string("unrecognized option: -") + *short_name);
            }
        }
        else
            args_.push_back(*arg); // just dash
    }
    else
        args_.push_back(*arg);

} // acmacs::argv::v2::argv::use

// ----------------------------------------------------------------------

void acmacs::argv::v2::argv::register_option(acmacs::argv::v2::detail::option_base* opt)
{
    options_.push_back(opt);

} // acmacs::argv::v2::argv::register_option

// ----------------------------------------------------------------------

acmacs::argv::v2::detail::option_base* acmacs::argv::v2::argv::find(char short_name)
{
    const auto found = std::find_if(options_.begin(), options_.end(), [short_name](const auto* opt) -> bool { return opt->short_name() == short_name; });
    if (found != options_.end())
        return *found;
    else
        return nullptr;

} // acmacs::argv::v2::argv::find

// ----------------------------------------------------------------------

acmacs::argv::v2::detail::option_base* acmacs::argv::v2::argv::find(std::string_view long_name)
{
    const auto found = std::find_if(options_.begin(), options_.end(), [long_name](const auto* opt) -> bool { return opt->long_name() == long_name; });
    if (found != options_.end())
        return *found;
    else
        return nullptr;

} // acmacs::argv::v2::argv::find

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
