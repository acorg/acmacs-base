#pragma once

#include <stdexcept>
#include <numeric>

#include "acmacs-base/line.hh"

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

    class StandardDeviation
    {
     public:
        constexpr double mean() const { return mean_; }
        constexpr double sd() const { return sd_; }

     private:
        StandardDeviation(double mean, double sd) : mean_{mean}, sd_{sd} {}

        double mean_;
        double sd_ = 0;

        template <typename ForwardIterator> friend StandardDeviation standard_deviation(ForwardIterator, ForwardIterator, double);
        template <typename ForwardIterator> friend StandardDeviation standard_deviation(ForwardIterator, ForwardIterator);
    };

    template <typename ForwardIterator> inline StandardDeviation standard_deviation(ForwardIterator first, ForwardIterator last, double mean)
    {
        return {mean, std::sqrt(varianceN(first, last, mean) / (last - first))};
    }

    template <typename ForwardIterator> inline StandardDeviation standard_deviation(ForwardIterator first, ForwardIterator last)
    {
        const auto [mean, size] = mean_size(first, last);
        return {mean, std::sqrt(varianceN(first, last, mean) / size)};
    }

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
        using LineDefinedByEquation::LineDefinedByEquation;
    };

    template <typename XForwardIterator, typename YForwardIterator> inline SimpleLinearRegression simple_linear_regression(XForwardIterator x_first, XForwardIterator x_last, YForwardIterator y_first)
    {
        const auto [x_mean, x_size] = mean_size(x_first, x_last);
        const auto y_mean = mean(y_first, x_size);
        const auto slope = covarianceN(x_first, x_last, x_mean, y_first, y_mean) / varianceN(x_first, x_last, x_mean);
        return {slope, y_mean - slope * x_mean};
    }

} // namespace acmacs::statistics

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
