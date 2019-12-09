#include "acmacs-base/time-series.hh"

// ----------------------------------------------------------------------

int main()
{
    int exit_code = 0;

    using year = date::year;
    const acmacs::time_series::series yearly_expected{{year{2016}/1/1, year{2017}/1/1}, {year{2017}/1/1, year{2018}/1/1}, {year{2018}/1/1, year{2019}/1/1,}};
    const acmacs::time_series::series monthly_expected{{year{2018}/11/1, year{2018}/12/1}, {year{2018}/12/1, year{2019}/1/1}, {year{2019}/1/1, year{2019}/2/1}, {year{2019}/2/1, year{2019}/3/1}, {year{2019}/3/1, year{2019}/4/1}};

    try {
        {
            const acmacs::time_series::parameters param{year{2016}/1/1, year{2019}/1/1, acmacs::time_series::interval::year};
            const auto ts = acmacs::time_series::make(param);
            if (ts != yearly_expected)
                throw std::runtime_error(fmt::format("Invalid result for {}:\n          {}\nexpected: {}\n", param, ts, yearly_expected));
        }
        {
            const acmacs::time_series::parameters param{year{2016}/1/10, year{2018}/1/10, acmacs::time_series::interval::year};
            const auto ts = acmacs::time_series::make(param);
            if (ts != yearly_expected)
                throw std::runtime_error(fmt::format("Invalid result for {}:\n          {}\nexpected: {}\n", param, ts, yearly_expected));
        }
        {
            const acmacs::time_series::parameters param{year{2016}/2/1, year{2018}/2/10, acmacs::time_series::interval::year};
            const auto ts = acmacs::time_series::make(param);
            if (ts != yearly_expected)
                throw std::runtime_error(fmt::format("Invalid result for {}:\n          {}\nexpected: {}\n", param, ts, yearly_expected));
        }

        // ----------------------------------------------------------------------

        {
            const acmacs::time_series::parameters param{year{2018}/11/10, year{2019}/4/1};
            const auto ts = acmacs::time_series::make(param);
            if (ts != monthly_expected)
                throw std::runtime_error(fmt::format("Invalid result for {}:\n          {}\nexpected: {}\n", param, ts, monthly_expected));
        }
        {
            const acmacs::time_series::parameters param{year{2018}/11/10, year{2019}/3/2};
            const auto ts = acmacs::time_series::make(param);
            if (ts != monthly_expected)
                throw std::runtime_error(fmt::format("Invalid result for {}:\n          {}\nexpected: {}\n", param, ts, monthly_expected));
        }

        // ----------------------------------------------------------------------

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
