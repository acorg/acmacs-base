#include "acmacs-base/argv.hh"
#include "acmacs-base/log.hh"
#include "acmacs-base/settings-v2.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<bool> verbose{*this, 'v', "verbose"};

    argument<str_array> settings_files{*this, arg_name{"settings.json"}};
};

int main(int argc, const char* argv[])
{
    using namespace std::string_view_literals;
    int exit_code = 0;

    try {
        Options opt(argc, argv);
        if (opt.verbose)
            acmacs::log::enable(acmacs::log::settings);
        if (!opt.settings_files.empty()) {
            acmacs::settings::v2::Settings s1;
            AD_DEBUG("loading {}", opt.settings_files);
            s1.load(opt.settings_files);
        }
        else {
            {
                AD_DEBUG("reading {}", "test-settings-v2.1.json");
                acmacs::settings::v2::Settings s2({"test-settings-v2.1.json"});
                s2.apply("main"sv);
            }
            {
                AD_DEBUG("reading {}", "test-settings-v2.2.json");
                acmacs::settings::v2::Settings s3({"test-settings-v2.2.json", "test-settings-v2.3.json"});
                s3.apply("main"sv);
            }
        }
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
        exit_code = 1;
    }

    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
