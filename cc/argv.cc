#include <iostream>
#include <algorithm>
#include <cstdlib>

#include "acmacs-base/argv.hh"

// ----------------------------------------------------------------------

// std::ostream& acmacs::argv::v2::operator<<(std::ostream& out, const argv& args)
// {
//     out << "argv0: " << args.prog_name_ << '\n';
//     for (const auto* opt : args.options_)
//         out << *opt << '\n';
//     for (const auto& arg : args.args_)
//         out << '"' << arg << "\"\n";
//     return out;

// } // acmacs::argv::v2::operator<<

// ----------------------------------------------------------------------

std::string acmacs::argv::v2::detail::option_base::names() const
{
    std::string result;
    if (short_name_) {
        result.append(1, '-');
        result.append(1, short_name_);
        if (!long_name_.empty())
            result.append(1, ',');
    }
    if (!long_name_.empty()) {
        result.append("--");
        result.append(long_name_);
    }
    return result;

} // acmacs::argv::v2::detail::option_base::names

// ----------------------------------------------------------------------

bool acmacs::argv::v2::argv::parse(int argc, const char* const argv[], acmacs::argv::v2::argv::on_error on_err)
{
    prog_name_ = argv[0];
    const std::vector command_line(argv + 1, argv + argc);
    for (auto arg = command_line.begin(); arg != command_line.end(); ++arg)
        use(arg);
    if (errors_.empty() && !show_help_)
        return true;
    else {
        switch (on_err) {
          case on_error::exit:
              for (const auto& err: errors_)
                  std::cerr << "ERROR: " << err << '\n';
              if (show_help_) {
                  std::cerr << '\n';
                  show_help(std::cerr);
              }
              std::exit(errors_.empty() ? 0 : 1);
          case on_error::raise:
              if (errors_.empty())
                  throw acmacs::argv::v2::show_help{};
              else
                  throw acmacs::argv::v2::errors{};
          case on_error::return_false:
              if (show_help_)
                  show_help(std::cerr);
              return errors_.empty();
        }
    }
    throw std::runtime_error{"argv::parse: internal"};

} // acmacs::argv::v2::argv::parse

// ----------------------------------------------------------------------

void acmacs::argv::v2::argv::show_help(std::ostream& out) const
{
    out << "Usage: " << prog_name_ << " [options]\n";
    for (const auto* opt : options_) {
        out << "  " << opt->names() << "  " << opt->description() << '\n';
    }

} // acmacs::argv::v2::argv::show_help

// ----------------------------------------------------------------------

void acmacs::argv::v2::argv::use(detail::cmd_line_iter& arg)
{
    if ((*arg)[0] == '-') {
        if ((*arg)[1] == '-') {
            if ((*arg)[2]) {    // long opt
                if (auto* opt = find(std::string_view(*arg + 2)); opt)
                    opt->add(arg);
                else
                    errors_.push_back(std::string("unrecognized option: ") + *arg);
            }
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
