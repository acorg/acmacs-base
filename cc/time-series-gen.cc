#include <iostream>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/time-series.hh"
#include "acmacs-base/rjson.hh"
#include "acmacs-base/read-file.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

template <typename TS> std::string gen(const Date& aStart, const Date& aEnd)
{
    rjson::array data;
    TS ts(aStart, aEnd);
    for (auto entry = ts.begin(); entry != ts.end(); ++entry) {
        data.insert(rjson::object{{
                    {"text_name", rjson::string{entry.text_name()}},
                    {"numeric_name", rjson::string{entry.numeric_name()}},
                    {"first_date", rjson::string{entry.first_date()}},
                    {"after_last_date", rjson::string{entry.after_last_date()}},
                }});
    }
    return data.to_json_pp(2);
}

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    int exit_code = 0;
    try {
        argc_argv args(argc, argv, {
                {"--verbose", false},
                {"-h", false},
                {"--help", false},
                {"-v", false},
                        });
        if (args["-h"] || args["--help"] || args.number_of_arguments() < 3) {
            std::cerr << "Usage: " << args.program() << " [options] <monthly|yearly|weekly> <start-date> <end-date> [<output.json>]\n" << args.usage_options() << '\n';
            exit_code = 1;
        }
        else {
            Date start, end;
            try {
                start = args[1];
            }
            catch (std::exception& err) {
                throw std::runtime_error("Cannot parse date from \""s + args[1] + "\": " + err.what());
            }
            try {
                end = args[2];
            }
            catch (std::exception& err) {
                throw std::runtime_error("Cannot parse date from \""s + args[2] + "\": " + err.what());
            }
            std::string json;
            if (args[0] == "monthly"s) {
                json = gen<MonthlyTimeSeries>(start, end);
            }
            else if (args[0] == "yearly"s) {
                json = gen<YearlyTimeSeries>(start, end);
            }
            else if (args[0] == "weekly"s) {
                json = gen<WeeklyTimeSeries>(start, end);
            }
            else {
                throw std::runtime_error("Unrecognized period: \""s + args[0] + "\", expected: monthly yearly weekly");
            }
            if (args.number_of_arguments() > 3)
                acmacs::file::write(args[3], json);
            else
                std::cout << json << '\n';
        }
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
