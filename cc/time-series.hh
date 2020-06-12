#pragma once

#include "acmacs-base/date.hh"
#include "acmacs-base/counter.hh"

// ----------------------------------------------------------------------

namespace acmacs::time_series::inline v2
{
    struct slot
    {
        date::year_month_day first, after_last;
        constexpr bool operator==(const slot& rhs) const { return first == rhs.first && after_last == rhs.after_last; }
        constexpr bool within(const date::year_month_day& date) const { return first <= date && date < after_last; }
    };

    using series = std::vector<slot>;

    enum class interval { year, month, week, day };

    interval interval_from_string(std::string_view interval_name);

    struct parameters
    {
        date::year_month_day first{date::invalid_date()}, after_last{date::invalid_date()};
        interval intervl{interval::month};
        date::period_diff_t number_of_intervals{1};

        parameters() = default;
        parameters(date::year_month_day a_first, date::year_month_day a_after_last) : first{a_first}, after_last{a_after_last} {}
        parameters(date::year_month_day a_first, date::year_month_day a_after_last, interval a_interval) : first{a_first}, after_last{a_after_last}, intervl{a_interval} {}
        parameters(date::year_month_day a_first, date::year_month_day a_after_last, interval a_interval, date::period_diff_t num) : first{a_first}, after_last{a_after_last}, intervl{a_interval}, number_of_intervals{num} {}
        parameters(std::string_view interval_name) : intervl{interval_from_string(interval_name)} {}
        parameters(std::string_view interval_name, date::period_diff_t num_intervals) : intervl{interval_from_string(interval_name)}, number_of_intervals{num_intervals} {}

        void update(std::optional<std::string_view> a_start, std::optional<std::string_view> a_end, std::optional<std::string_view> a_interval, std::optional<date::period_diff_t> a_number_of_intervals);
    };

    // parameters& update(const rjson::value& source, parameters& param);

    series make(const parameters& param);
    inline series make(date::year_month_day first, date::year_month_day after_last) { return make(parameters{first, after_last}); }
    inline series make(std::string_view first, std::string_view after_last) { return make(parameters{date::from_string(first, date::allow_incomplete::yes), date::from_string(after_last, date::allow_incomplete::yes)}); }

    std::string text_name(const parameters& param, const slot& a_slot);
    std::string text_name(const slot& a_slot);
    std::string numeric_name(const parameters& param, const slot& a_slot);
    std::string numeric_name(const slot& a_slot);
    std::string range_name(const parameters& param, const series& ser);

     // returns slot number or ser.size() if not found
    size_t find(const series& ser, const date::year_month_day& dat);

    using date_stat_t = acmacs::Counter<date::year_month_day>;
    std::pair<date::year_month_day, date::year_month_day> suggest_start_end(const parameters& param, const date_stat_t& stat);

    namespace detail
    {
        date::year_month_day first(const date::year_month_day& current, interval intervl);
        date::year_month_day next(const date::year_month_day& current, interval intervl);

    } // namespace detail

    template <typename Dates> date_stat_t stat(const parameters& param, const Dates& dates)
    {
        date_stat_t counter;
        for (const auto& dat : dates) {
            try {
                counter.count(detail::first(date::from_string(dat, date::allow_incomplete::yes), param.intervl));
            }
            catch (date::date_parse_error&) {
            }
        }
        return counter;
    }
}

// ----------------------------------------------------------------------

template <> struct fmt::formatter<acmacs::time_series::series> : public fmt::formatter<acmacs::fmt_helper::default_formatter>
{
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
        return fmt::formatter<std::string>::format("month", ctx); // to make g++-9 happy
    }
};

template <> struct fmt::formatter<acmacs::time_series::parameters> : public fmt::formatter<acmacs::fmt_helper::default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::time_series::parameters& param, FormatContext& ctx)
    {
        return format_to(ctx.out(), "time-series{{{} .. {}, {}:{}}}", param.first, param.after_last, param.intervl, param.number_of_intervals);
    }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
