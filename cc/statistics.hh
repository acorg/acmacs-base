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
        using LineDefinedByEquation::LineDefinedByEquation;
    };

    template <typename XForwardIterator, typename YForwardIterator> inline SimpleLinearRegression simple_linear_regression(XForwardIterator x_first, XForwardIterator x_last, YForwardIterator y_first)
    {
        const auto [x_mean, x_size] = mean_size(x_first, x_last);
        const auto y_mean = mean(y_first, x_size);
        const auto slope = covarianceN(x_first, x_last, x_mean, y_first, y_mean) / varianceN(x_first, x_last, x_mean);
        return {slope, y_mean - slope * x_mean};
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
        return covarianceN(x_first, x_last, x_mean, y_first, y_mean) / standard_deviation(x_first, x_last, x_mean).sd() / standard_deviation(y_first, y_first + size, y_mean).sd();
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
