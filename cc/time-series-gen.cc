#include "acmacs-base/argv.hh"
#include "acmacs-base/time-series.hh"
#include "acmacs-base/rjson-v2.hh"
#include "acmacs-base/read-file.hh"

// ----------------------------------------------------------------------

inline std::string gen(acmacs::time_series::parameters& param)
{
    rjson::v2::array data;
    for (const auto& slot : acmacs::time_series::make(param)) {
        data.append(rjson::v2::object{
                    {"text_name", date::monthtext_year(slot.first)},
                    {"numeric_name", date::display(slot.first, "%Y-%m")},
                    {"first_date", date::display(slot.first)},
                    {"after_last_date", date::display(slot.after_last)},
                });
    }
    return rjson::v2::pretty(data);
}

// ----------------------------------------------------------------------

using namespace acmacs::argv;

struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    argument<str> intervl{*this, arg_name{"monthly|yearly|weekly|daily"}, mandatory};
    argument<str> start{*this, arg_name{"start-date"}, mandatory};
    argument<str> end{*this, arg_name{"end-date"}, mandatory};
    argument<str> output{*this, arg_name{"output.json"}, dflt{""}};
};

int main(int argc, char* const argv[])
{
    int exit_code = 0;
    try {
        Options opt(argc, argv);
        acmacs::time_series::parameters param{date::from_string(opt.start), date::from_string(opt.end), acmacs::time_series::interval_from_string(opt.intervl)};
        const auto json = gen(param);
        if (opt.output)
            acmacs::file::write(opt.output, json);
        else
            fmt::print("{}\n", json);
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
