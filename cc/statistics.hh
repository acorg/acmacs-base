#pragma once

#include <stdexcept>
#include <numeric>

#include "acmacs-base/line.hh"
#include "acmacs-base/string.hh"

// ----------------------------------------------------------------------

namespace acmacs::statistics
{
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

// ----------------------------------------------------------------------

    template <typename ForwardIterator> inline double mean(ForwardIterator first, ForwardIterator last)
    {
        size_t num = 0;
        double sum = 0;
        for (; first != last; ++first, ++num)
            sum += *first;
        if (num == 0)
            throw Error("mean(): empty range");
        return sum / num;
    }

    template <typename Container> inline double mean(const Container& cont) { return mean(std::begin(cont), std::end(cont)); }

    template <typename ForwardIterator> inline double mean_abs(ForwardIterator first, ForwardIterator last)
    {
        size_t num = 0;
        double sum = 0;
        for (; first != last; ++first, ++num)
            sum += std::abs(*first);
        if (num == 0)
            throw Error("mean_abs(): empty range");
        return sum / num;
    }

    template <typename Container> inline double mean_abs(const Container& cont) { return mean_abs(std::begin(cont), std::end(cont)); }

    template <typename ForwardIterator> inline double mean(ForwardIterator first, size_t size)
    {
        if (size == 0)
            throw Error("mean(): empty range");
        double sum = 0;
        for (size_t no = 0; no < size; ++no, ++first)
            sum += *first;
        return sum / size;
    }

    template <typename ForwardIterator> inline std::pair<double, size_t> mean_size(ForwardIterator first, ForwardIterator last)
    {
        size_t num = 0;
        double sum = 0;
        for (; first != last; ++first, ++num)
            sum += *first;
        if (num == 0)
            throw Error("mean(): empty range");
        return {sum / num, num};
    }

// ----------------------------------------------------------------------

    template <typename ForwardIterator> inline double varianceN(ForwardIterator first, ForwardIterator last, double mean)
    {
        return std::accumulate(first, last, 0.0, [mean](double sum, double value) { return sum + sqr(value - mean); });
    }

    template <typename ForwardIterator> inline double varianceN(ForwardIterator first, ForwardIterator last)
    {
        return varianceN(first, last, mean(first, last));
    }

    template <typename XForwardIterator, typename YForwardIterator> inline double covarianceN(XForwardIterator x_first, XForwardIterator x_last, double x_mean, YForwardIterator y_first, double y_mean)
    {
        double sum = 0;
        for (; x_first != x_last; ++x_first, ++y_first)
            sum += (*x_first - x_mean) * (*y_first - y_mean);
        return sum;
    }

// ----------------------------------------------------------------------

    // https://en.wikipedia.org/wiki/Standard_deviation
    class StandardDeviation
    {
     public:
        constexpr double mean() const { return mean_; }
        constexpr double sample_sd() const { return sample_sd_; } // R sd() function returns sample sd
        constexpr double population_sd() const { return population_sd_; }

     private:
        StandardDeviation(double mean, double population_sd, double sample_sd) : mean_{mean}, population_sd_{population_sd}, sample_sd_{sample_sd} {}

        double mean_;
        double population_sd_ = 0;
        double sample_sd_;

        template <typename ForwardIterator> friend StandardDeviation standard_deviation(ForwardIterator, ForwardIterator, double);
        template <typename ForwardIterator> friend StandardDeviation standard_deviation(ForwardIterator, ForwardIterator);
    };

    template <typename ForwardIterator> inline StandardDeviation standard_deviation(ForwardIterator first, ForwardIterator last, double mean)
    {
        const auto vari = varianceN(first, last, mean);
        const auto size = last - first;
        return {mean, std::sqrt(vari / size), std::sqrt(vari / (size - 1))};
    }

    template <typename ForwardIterator> inline StandardDeviation standard_deviation(ForwardIterator first, ForwardIterator last)
    {
        return standard_deviation(first, last, mean(first, last));
    }

    template <typename Container> inline StandardDeviation standard_deviation(const Container& cont) { return standard_deviation(std::begin(cont), std::end(cont)); }

    // template <typename ForwardIterator> inline StandardDeviation standard_deviation(ForwardIterator first, ForwardIterator last, double mean)
    // {
    //     StandardDeviation result(mean);
    //     const auto sum_of_squares = std::inner_product(first, last, first, 0.0, std::plus<double>(), [m = result.mean_](double xx, double yy) { return (xx - m) * (yy - m); });
    //     result.sd_ = std::sqrt(sum_of_squares / (last - first));
    //     return result;
    // }

    // template <typename ForwardIterator> inline StandardDeviation standard_deviation(ForwardIterator first, ForwardIterator last)
    // {
    //     return standard_deviation(first, last, mean(first, last));
    // }

// ----------------------------------------------------------------------

    class SimpleLinearRegression : public LineDefinedByEquation
    {
     public:
        SimpleLinearRegression(double a_slope, double a_intercept, double a_r2, double a_rbar2) : LineDefinedByEquation(a_slope, a_intercept), r2_{a_r2}, rbar2_{a_rbar2} {}

        constexpr double c0() const { return intercept(); }
        constexpr double c1() const { return slope(); }
        constexpr double r2() const { return r2_; } // Coefficient of determination http://en.wikipedia.org/wiki/Coefficient_of_determination
        constexpr double rbar2() const { return rbar2_; }

      private:
        const double r2_, rbar2_;
    };

    inline std::ostream& operator<<(std::ostream& out, const SimpleLinearRegression& lrg) { return out << "LinearRegression(c0:" << lrg.c0() << ", c1:" << lrg.c1() << ", r2:" << lrg.r2() << ", rbar2:" << lrg.rbar2() << ')'; }

    // Adopted from GNU Scientific library (gsl_fit_linear)
    template <typename XForwardIterator, typename YForwardIterator> inline SimpleLinearRegression simple_linear_regression(XForwardIterator x_first, XForwardIterator x_last, YForwardIterator y_first)
    {
        const auto [x_mean, x_size] = mean_size(x_first, x_last);
        const auto y_mean = mean(y_first, x_size);
        const auto slope = covarianceN(x_first, x_last, x_mean, y_first, y_mean) / varianceN(x_first, x_last, x_mean);
        const auto intercept = y_mean - slope * x_mean;

        // SSE: sum of squared residuals
        double sse = 0.0;
        for (auto [xi, yi] = std::pair(x_first, y_first); xi != x_last; ++xi, ++yi)
            sse += sqr(*yi - (intercept + slope * (*xi)));

        // SST:  total sum of squares http://en.wikipedia.org/wiki/Total_sum_of_squares
        const auto sst = varianceN(y_first, y_first + static_cast<typename XForwardIterator::difference_type>(x_size), y_mean);
        if (sst <= 0)
            throw std::runtime_error(::string::concat("simple_linear_regression: cannot calculate R2: SST is wrong: ", sst));
        const auto r2 = 1.0 - sse / sst;
        const auto rbar2 = 1.0 - (1.0 - r2) * double(x_size - 1) / double(x_size - 2);

        return {slope, intercept, r2, rbar2};
    }

// ----------------------------------------------------------------------

    // http://en.wikipedia.org/wiki/Correlation
    // https://en.wikipedia.org/wiki/Pearson_correlation_coefficient
    template <typename XForwardIterator, typename YForwardIterator> inline double correlation(XForwardIterator x_first, XForwardIterator x_last, YForwardIterator y_first)
    {
        if (x_first == x_last)
            return 0.0;
        const auto size = x_last - x_first;
        const auto x_mean = mean(x_first, x_last), y_mean = mean(y_first, static_cast<size_t>(size));
        return covarianceN(x_first, x_last, x_mean, y_first, y_mean) / (size - 1) / standard_deviation(x_first, x_last, x_mean).sample_sd() / standard_deviation(y_first, y_first + size, y_mean).sample_sd();
    }

    template <typename Container> inline double correlation(const Container& first, const Container& second)
    {
        if (first.size() != second.size())
            throw std::invalid_argument("correlation: containers of different sizes");
        return correlation(std::begin(first), std::end(first), std::begin(second));
    }

} // namespace acmacs::statistics

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
