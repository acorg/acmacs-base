#include <iostream>

#include "acmacs-base/argv.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/time-series.hh"
#include "acmacs-base/rjson.hh"
#include "acmacs-base/read-file.hh"

// ----------------------------------------------------------------------

template <typename TS> std::string gen(const date::year_month_day& aStart, const date::year_month_day& aEnd)
{
    rjson::v2::array data;
    TS ts(aStart, aEnd);
    for (auto entry = ts.begin(); entry != ts.end(); ++entry) {
        data.append(rjson::v2::object{
                    {"text_name", entry.text_name()},
                    {"numeric_name", entry.numeric_name()},
                    {"first_date", entry.first_date()},
                    {"after_last_date", entry.after_last_date()},
                });
    }
    return rjson::v2::pretty(data);
}

// ----------------------------------------------------------------------

using namespace acmacs::argv;

struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    argument<str> period{*this, arg_name{"monthly|yearly|weekly"}, mandatory};
    argument<str> start{*this, arg_name{"start-date"}, mandatory};
    argument<str> end{*this, arg_name{"end-date"}, mandatory};
    argument<str> output{*this, arg_name{"output.json"}, dflt{""}};
};

int main(int argc, char* const argv[])
{
    int exit_code = 0;
    try {
        Options opt(argc, argv);
        std::string json;
        if (opt.period == "monthly") {
            json = gen<MonthlyTimeSeries>(date::from_string(*opt.start), date::from_string(*opt.end));
        }
        else if (opt.period == "yearly") {
            json = gen<YearlyTimeSeries>(date::from_string(*opt.start), date::from_string(*opt.end));
        }
        else if (opt.period == "weekly") {
            json = gen<WeeklyTimeSeries>(date::from_string(*opt.start), date::from_string(*opt.end));
        }
        else {
            throw std::runtime_error(string::concat("Unrecognized period: \"", static_cast<str>(opt.period), "\", expected: monthly yearly weekly"));
        }
        if (opt.output.has_value())
            acmacs::file::write(opt.output, json);
        else
            std::cout << json << '\n';
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
