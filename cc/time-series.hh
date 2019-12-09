#pragma once

#include <vector>

#include <iterator>

#include "acmacs-base/date.hh"

// ----------------------------------------------------------------------

namespace rjson::inline v2
{
    class value;
}

namespace acmacs::time_series::inline v2
{
    struct slot
    {
        date::year_month_day first, after_last;
        constexpr bool operator==(const slot& rhs) const { return first == rhs.first && after_last == rhs.after_last; }
    };

    using series = std::vector<slot>;

    enum class interval { year, month, week, day };

    interval interval_from_string(std::string_view interval_name);

    struct parameters
    {
        date::year_month_day first{date::invalid_date()}, after_last{date::invalid_date()};
        interval intervl{interval::month};
        size_t number_of_intervals{1};
    };

    parameters& update(const rjson::value& source, parameters& param);

    series make(const parameters& param);
    inline series make(date::year_month_day first, date::year_month_day after_last) { return make(parameters{first, after_last}); }
    inline series make(std::string_view first, std::string_view after_last) { return make(parameters{date::from_string(first, date::allow_incomplete::yes), date::from_string(after_last, date::allow_incomplete::yes)}); }

    std::string text_name(const slot& a_slot);
    std::string numeric_name(const slot& a_slot);

     // returns slot number or ser.size() if not found
    size_t find(const series& ser, const date::year_month_day& dat);
}

// ----------------------------------------------------------------------

template <> struct fmt::formatter<acmacs::time_series::series>
{
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }
    template <typename FormatContext> auto format(const acmacs::time_series::series& series, FormatContext& ctx)
    {
        format_to(ctx.out(), "time-series{{");
        for (const auto& slot : series)
            format_to(ctx.out(), " {}..{}", slot.first, slot.after_last);
        return format_to(ctx.out(), "}}");
    }
};

template <> struct fmt::formatter<acmacs::time_series::interval> : fmt::formatter<std::string> {
    template <typename FormatCtx> auto format(const acmacs::time_series::interval& intervl, FormatCtx& ctx)
    {
        switch (intervl) {
            case acmacs::time_series::v2::interval::year:
                return fmt::formatter<std::string>::format("year", ctx);
            case acmacs::time_series::v2::interval::month:
                return fmt::formatter<std::string>::format("month", ctx);
            case acmacs::time_series::v2::interval::week:
                return fmt::formatter<std::string>::format("week", ctx);
            case acmacs::time_series::v2::interval::day:
                return fmt::formatter<std::string>::format("day", ctx);
        }
    }
};

template <> struct fmt::formatter<acmacs::time_series::parameters>
{
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }
    template <typename FormatContext> auto format(const acmacs::time_series::parameters& param, FormatContext& ctx)
    {
        return format_to(ctx.out(), "time-series{{{} .. {}, {}:{}}}", param.first, param.after_last, param.intervl, param.number_of_intervals);
    }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
