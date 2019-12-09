#include "acmacs-base/time-series.hh"

// ----------------------------------------------------------------------

int main()
{
    int exit_code = 0;

    using year = date::year;
    const acmacs::time_series::series yearly_expected{{year{2016}/1/1, year{2017}/1/1}, {year{2017}/1/1, year{2018}/1/1}, {year{2018}/1/1, year{2019}/1/1,}};
    const acmacs::time_series::series monthly1_expected{{year{2018}/11/1, year{2018}/12/1}, {year{2018}/12/1, year{2019}/1/1}, {year{2019}/1/1, year{2019}/2/1}, {year{2019}/2/1, year{2019}/3/1}, {year{2019}/3/1, year{2019}/4/1}};
    const acmacs::time_series::series monthly2_expected{{year{2018}/11/1, year{2019}/1/1}, {year{2019}/1/1, year{2019}/3/1}, {year{2019}/3/1, year{2019}/5/1}};
    const acmacs::time_series::series weekly1_expected{{year{2019}/1/1, year{2019}/1/8}, {year{2019}/1/8, year{2019}/1/15}, {year{2019}/1/15, year{2019}/1/22}, {year{2019}/1/22, year{2019}/1/29}, {year{2019}/1/29, year{2019}/2/5}, {year{2019}/2/5, year{2019}/2/12}, {year{2019}/2/12, year{2019}/2/19}, {year{2019}/2/19, year{2019}/2/26}, {year{2019}/2/26, year{2019}/3/5}};
    const acmacs::time_series::series weekly1l_expected{{year{2020}/1/1, year{2020}/1/8}, {year{2020}/1/8, year{2020}/1/15}, {year{2020}/1/15, year{2020}/1/22}, {year{2020}/1/22, year{2020}/1/29}, {year{2020}/1/29, year{2020}/2/5}, {year{2020}/2/5, year{2020}/2/12}, {year{2020}/2/12, year{2020}/2/19}, {year{2020}/2/19, year{2020}/2/26}, {year{2020}/2/26, year{2020}/3/4}};

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
            if (ts != monthly1_expected)
                throw std::runtime_error(fmt::format("Invalid result for {}:\n          {}\nexpected: {}\n", param, ts, monthly1_expected));
        }
        {
            const acmacs::time_series::parameters param{year{2018}/11/10, year{2019}/3/2};
            const auto ts = acmacs::time_series::make(param);
            if (ts != monthly1_expected)
                throw std::runtime_error(fmt::format("Invalid result for {}:\n          {}\nexpected: {}\n", param, ts, monthly1_expected));
        }
        {
            const acmacs::time_series::parameters param{year{2018}/11/10, year{2019}/4/1, acmacs::time_series::interval::month, 2};
            const auto ts = acmacs::time_series::make(param);
            if (ts != monthly2_expected)
                throw std::runtime_error(fmt::format("Invalid result for {}:\n          {}\nexpected: {}\n", param, ts, monthly2_expected));
        }

        // ----------------------------------------------------------------------

        {
            const acmacs::time_series::parameters param{year{2019}/1/1, year{2019}/3/1, acmacs::time_series::interval::week};
            const auto ts = acmacs::time_series::make(param);
            if (ts != weekly1_expected)
                throw std::runtime_error(fmt::format("Invalid result for {}:\n          {}\nexpected: {}\n", param, ts, weekly1_expected));
        }
        {
            const acmacs::time_series::parameters param{year{2020}/1/1, year{2020}/3/1, acmacs::time_series::interval::week};
            const auto ts = acmacs::time_series::make(param);
            if (ts != weekly1l_expected)
                throw std::runtime_error(fmt::format("Invalid result for {}:\n          {}\nexpected: {}\n", param, ts, weekly1l_expected));
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
