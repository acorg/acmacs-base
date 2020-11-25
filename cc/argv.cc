#include <algorithm>
#include <numeric>
#include <cstdlib>

#include "acmacs-base/argv.hh"
#include "acmacs-base/acmacsd.hh"
#include "acmacs-base/string-join.hh"

// ----------------------------------------------------------------------

std::string acmacs::argv::v2::detail::option_base::names() const noexcept
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
    if (has_arg()) {
        result.append(1, ' ');
        result.append(arg_name_);
    }
    return result;

} // acmacs::argv::v2::detail::option_base::names

// ----------------------------------------------------------------------

bool acmacs::argv::v2::argv::parse(int argc, const char* const argv[], acmacs::argv::v2::argv::on_error on_err)
{
    argv0_ = argv[0];
    if (const auto last_slash_pos = argv0_.rfind('/'); last_slash_pos != std::string_view::npos)
        prog_name_ = argv0_.substr(last_slash_pos + 1);
    else
        prog_name_ = argv0_;
    const std::vector command_line(argv + 1, argv + argc);
    for (auto arg = command_line.begin(); arg != command_line.end(); ) {
        use(arg, command_line.end());
        if (arg != command_line.end())
            ++arg;
    }
    auto specified_arg = args_.begin();
    for (auto opt = options_.begin(); opt != options_.end() && specified_arg != args_.end(); ++opt) {
        if (!(*opt)->has_name()) {
            if ((*opt)->multiple_values()) {
                for (; specified_arg != args_.end(); ++specified_arg)
                    (*opt)->add(*specified_arg);
            }
            else {
                (*opt)->add(*specified_arg);
                ++specified_arg;
            }
        }
    }
    if (specified_arg != args_.end())
        errors_.push_back(std::to_string(args_.end() - specified_arg) + " extra argument(s) given at the command line");
    for (const auto* opt : options_) {
        if (opt->mandatory() && !opt->has_value())
            errors_.push_back(std::string("mandatory ") + (opt->has_name() ? "switch" : "argument") + " not given: " + opt->names());
    }
    if (errors_.empty() && !show_help_) {
        return true;
    }
    else {
        switch (on_err) {
          case on_error::exit:
              for (const auto& err: errors_)
                  AD_ERROR("{}", err);
              fmt::print(stderr, "\n");
              show_help();
              std::exit(errors_.empty() ? 0 : 1);
          case on_error::raise:
              throw acmacs::argv::v2::show_help{fmt::format("{}\n\n{}", acmacs::string::join(acmacs::string::join_newline, errors_), format_help())};
          case on_error::return_false:
              if (show_help_)
                  show_help();
              return errors_.empty();
        }
    }
    throw std::runtime_error{"argv::parse: internal"};

} // acmacs::argv::v2::argv::parse

// ----------------------------------------------------------------------

void acmacs::argv::v2::argv::show_help() const
{
        fmt::print(stderr, "{}", format_help());

} // acmacs::argv::v2::argv::show_help

// ----------------------------------------------------------------------

std::string acmacs::argv::v2::argv::format_help() const
{
    try {
        fmt::memory_buffer out;

        const size_t name_width = std::accumulate(options_.begin(), options_.end(), 0UL, [](size_t width, const auto* opt) { return std::max(width, opt->names().size()); });
        if (const auto pre = help_pre(); !pre.empty())
            fmt::format_to(out, "{}\n", pre);
        fmt::format_to(out, "Usage: {} [options]", prog_name_);
        for (const auto* opt : options_) {
            if (!opt->has_name()) {
                fmt::format_to(out, " {}{}", opt->mandatory() ? '<' : '[', opt->arg_name());
                if (const auto dflt = opt->get_default(); !dflt.empty())
                    fmt::format_to(out, ": {}", dflt);
                fmt::format_to(out, "{}", opt->mandatory() ? '>' : ']');
                if (opt->multiple_values())
                    fmt::format_to(out, " ...");
            }
        }
        fmt::format_to(out, "\n\n");
        if (!descriptions_.empty()) {
            for (const auto* desc : descriptions_)
                fmt::format_to(out, "{}\n", desc->text());
        }
        fmt::format_to(out, "\n");
        for (const auto* opt : options_) {
            if (opt->has_name()) {
                fmt::format_to(out, "  {:<{}s}", opt->names(), name_width);
                if (opt->mandatory())
                    fmt::format_to(out, " (MANDATORY)");
                if (const auto dflt = opt->get_default(); !opt->is_bool() && !dflt.empty())
                    fmt::format_to(out, " (def: {})", dflt);
                // fmt::format_to(out, " {}\n", fmt::format(opt->description(), fmt::arg("ACMACSD_ROOT", acmacsd_root())));
                fmt::format_to(out, " {}\n", opt->description());
            }
        }
        if (const auto post = help_post(); !post.empty())
            fmt::format_to(out, "{}\n", post);

        return fmt::to_string(out);
    }
    catch (std::exception& err) {
        AD_ERROR("argv: help formatting failed: {}", err);
        throw;
    }

} // acmacs::argv::v2::argv::format_help

// ----------------------------------------------------------------------

void acmacs::argv::v2::argv::use(detail::cmd_line_iter& arg, detail::cmd_line_iter last)
{
    const detail::cmd_line_iter arg_save = arg;
    try {
        if ((*arg)[0] == '-') {
            if ((*arg)[1] == '-') {
                if ((*arg)[2]) { // long opt
                    if (auto* opt = find(std::string_view(*arg + 2)); opt)
                        opt->add(arg, last);
                    else
                        errors_.push_back(fmt::format("unrecognized option: {}", *arg));
                }
                else
                    args_.push_back(*arg); // just two dashes
            }
            else if ((*arg)[1]) { // short opts
                for (const char* short_name = *arg + 1; *short_name; ++short_name) {
                    if (auto* opt = find(*short_name); opt)
                        opt->add(arg, last);
                    else
                        errors_.push_back(fmt::format("unrecognized option: -{}", *short_name));
                }
            }
            else
                args_.push_back(*arg); // just dash
        }
        else
            args_.push_back(*arg);
    }
    catch (detail::invalid_option_value& err) {
        errors_.push_back(std::string(*arg_save) + ": " + err.what());
    }

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
