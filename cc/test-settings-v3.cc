#include "acmacs-base/settings-v3-test.hh"
#include "acmacs-base/argv.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;

struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<bool> verbose{*this, 'v'};

    argument<str> filename{*this, arg_name{"file.json"}, mandatory};
    argument<str> name{*this, arg_name{"name-to-apply"}, mandatory};
};

int main(int argc, char* argv[])
{
    Options opt(argc, argv);

    acmacs::log::register_enabler_acmacs_base();
    if (opt.verbose)
        acmacs::log::enable("settings");

    acmacs::settings::v3::TestData data;
    data.load(opt.filename);
    data.apply(opt.name);

    return 0;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
