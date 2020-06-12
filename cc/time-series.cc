#include "acmacs-base/time-series.hh"
#include "acmacs-base/debug.hh"
#include "acmacs-base/enumerate.hh"

// ----------------------------------------------------------------------

date::year_month_day acmacs::time_series::v2::detail::next(const date::year_month_day& current, acmacs::time_series::v2::interval intervl)
{
    switch (intervl) {
        case acmacs::time_series::v2::interval::year:
            return date::next_year(current);
        case acmacs::time_series::v2::interval::month:
            return date::next_month(current);
        case acmacs::time_series::v2::interval::week:
            return date::next_week(current);
        case acmacs::time_series::v2::interval::day:
            return date::next_day(current);
    }
    return date::next_month(current); // to make g++-9 happy

} // acmacs::time_series::v2::detail::next

// ----------------------------------------------------------------------

date::year_month_day acmacs::time_series::v2::detail::first(const date::year_month_day& current, acmacs::time_series::v2::interval intervl)
{
    switch (intervl) {
        case acmacs::time_series::v2::interval::year:
            return date::beginning_of_year(current);
        case acmacs::time_series::v2::interval::month:
            return date::beginning_of_month(current);
        case acmacs::time_series::v2::interval::week:
            return current; // date::beginning_of_week(current);
        case acmacs::time_series::v2::interval::day:
            return current;
    }
    return date::beginning_of_month(current); // to make g++-9 happy

} // acmacs::time_series::v2::detail::next

// ----------------------------------------------------------------------

acmacs::time_series::v2::series acmacs::time_series::v2::make(const parameters& param)
{
    const auto increment = [](const date::year_month_day& cur, interval intervl, auto count) {
        auto result = cur;
        for (decltype(count) i = 0; i < count; ++i)
            result = detail::next(result, intervl);
        return result;
    };

    series result;
    for (auto current = detail::first(param.first, param.intervl); current < param.after_last; ) {
        const auto subsequent = increment(current, param.intervl, param.number_of_intervals);
        result.push_back({current, subsequent});
        current = subsequent;
    }
    return result;

} // acmacs::time_series::v2::make

// ----------------------------------------------------------------------

acmacs::time_series::v2::interval acmacs::time_series::v2::interval_from_string(std::string_view interval_name)
{
    using namespace std::string_view_literals;
    if (interval_name == "monthly"sv || interval_name == "month"sv)
        return acmacs::time_series::interval::month;
    else if (interval_name == "yearly"sv || interval_name == "year"sv)
        return acmacs::time_series::interval::year;
    else if (interval_name == "weekly"sv || interval_name == "week"sv)
        return acmacs::time_series::interval::week;
    else if (interval_name == "daily"sv || interval_name == "day"sv)
        return acmacs::time_series::interval::day;
    else {
        AD_WARNING("unrecognized interval specification: \"{}\", month assumed", interval_name);
        return acmacs::time_series::interval::month;
    }

} // acmacs::time_series::v2::interval_from_string

// ----------------------------------------------------------------------

void acmacs::time_series::v2::parameters::update(std::optional<std::string_view> a_start, std::optional<std::string_view> a_end, std::optional<std::string_view> a_interval,
                                                 std::optional<date::period_diff_t> a_number_of_intervals)
{
    if (a_start.has_value())
        first = date::from_string(*a_start, date::allow_incomplete::yes);
    if (a_end.has_value())
        after_last = date::from_string(*a_end, date::allow_incomplete::yes);
    if (a_interval.has_value()) {
        intervl = interval_from_string(*a_interval);
        if (a_number_of_intervals.has_value())
            number_of_intervals = *a_number_of_intervals;
    }

} // acmacs::time_series::v2::parameters::update

// ----------------------------------------------------------------------


// acmacs::time_series::v2::parameters& acmacs::time_series::v2::update(const rjson::v2::value& source, parameters& param)
// {
//     using namespace std::string_view_literals;
//     if (const auto& start = source["start"sv]; !start.is_null())
//         param.first = date::from_string(start.to<std::string_view>(), date::allow_incomplete::yes);
//     if (const auto& end = source["end"sv]; !end.is_null())
//         param.after_last = date::from_string(end.to<std::string_view>(), date::allow_incomplete::yes);
//     std::visit(
//         [&param]<typename T>(T && arg) {
//             if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
//                 param.intervl = interval_from_string(arg);
//             }
//             else if constexpr (std::is_same_v<std::decay_t<T>, rjson::v2::object>) {
//                 if (const auto& mon = arg.get("month"sv); !mon.is_null()) {
//                     param.intervl = interval::month;
//                     param.number_of_intervals = mon.template to<size_t>();
//                 }
//                 else if (const auto& yea = arg.get("year"sv); !yea.is_null()) {
//                     param.intervl = interval::year;
//                     param.number_of_intervals = yea.template to<size_t>();
//                 }
//                 else if (const auto& wee = arg.get("week"sv); !wee.is_null()) {
//                     param.intervl = interval::week;
//                     param.number_of_intervals = wee.template to<size_t>();
//                 }
//                 else if (const auto& da = arg.get("day"sv); !da.is_null()) {
//                     param.intervl = interval::day;
//                     param.number_of_intervals = da.template to<size_t>();
//                 }
//                 else {
//                     fmt::print(stderr, "WARNING: unrecognized interval specification: {}\n", arg);
//                 }
//             }
//             else if constexpr (!std::is_same_v<std::decay_t<T>, rjson::v2::const_null> && !std::is_same_v<std::decay_t<T>, rjson::v2::null>) {
//                 fmt::print(stderr, "WARNING: unrecognized interval specification: {}\n", arg);
//             }
//         },
//         source["interval"].val_());
//     return param;

// } // acmacs::time_series::v2::make

// ----------------------------------------------------------------------

size_t acmacs::time_series::v2::find(const series& ser, const date::year_month_day& dat)
{
    for (auto [no, slot] : acmacs::enumerate(ser)) {
        if (dat >= slot.first && dat < slot.after_last)
            return no;
    }
    return ser.size();

} // acmacs::time_series::v2::find

// ----------------------------------------------------------------------

std::string acmacs::time_series::v2::text_name(const parameters& param, const slot& a_slot)
{
    switch (param.intervl) {
        case interval::year:
            return date::year_4(a_slot.first);
        case interval::month:
            if (param.number_of_intervals == 1) {
                return date::monthtext_year(a_slot.first);
            }
            else {
                const auto last{date::days_ago(a_slot.after_last, 1)};
                if (date::get_year(a_slot.first) == date::get_year(last))
                    return fmt::format("{} - {} {}", date::monthtext(a_slot.first), date::monthtext(last), date::year_4(a_slot.first));
                else
                    return fmt::format("{} {} - {} {}", date::monthtext(a_slot.first), date::year_4(a_slot.first), date::monthtext(last), date::year_4(last));
            }
        case interval::week:
        case interval::day:
            return date::display(a_slot.first);
    }
    return date::display(a_slot.first); // g++9

} // acmacs::time_series::v2::text_name

std::string acmacs::time_series::v2::text_name(const slot& a_slot)
{
    if (const auto days = date::days_between_dates(a_slot.first, a_slot.after_last); days < 28)
        return date::display(a_slot.first);
    else if (days < 45)
        return date::monthtext_year(a_slot.first);
    else if (days < 360) {
        const auto last{date::days_ago(a_slot.after_last, 1)};
        if (date::get_year(a_slot.first) == date::get_year(last))
            return fmt::format("{} - {} {}", date::monthtext(a_slot.first), date::monthtext(last), date::year_4(a_slot.first));
        else
            return fmt::format("{} {} - {} {}", date::monthtext(a_slot.first), date::year_4(a_slot.first), date::monthtext(last), date::year_4(last));
    }
    else
        return date::year_4(a_slot.first);

} // acmacs::time_series::v2::text_name

// ----------------------------------------------------------------------

std::string acmacs::time_series::v2::numeric_name(const parameters& param, const slot& a_slot)
{
    switch (param.intervl) {
        case interval::year:
            return date::year_4(a_slot.first);
        case interval::month:
            if (param.number_of_intervals == 1) {
                return date::year4_month2(a_slot.first);
            }
            else
                return fmt::format("{} - {}", date::year4_month2(a_slot.first), date::year4_month2(date::days_ago(a_slot.after_last, 1)));
        case interval::week:
        case interval::day:
            return date::display(a_slot.first);
    }
    return date::display(a_slot.first); // g++9

} // acmacs::time_series::v2::numeric_name

std::string acmacs::time_series::v2::numeric_name(const slot& a_slot)
{
    if (const auto days = date::days_between_dates(a_slot.first, a_slot.after_last); days < 28)
        return date::display(a_slot.first);
    else if (days < 360)
        return date::year4_month2(a_slot.first);
    else
        return date::year_4(a_slot.first);

} // acmacs::time_series::v2::numeric_name

// ----------------------------------------------------------------------

std::string acmacs::time_series::v2::range_name(const parameters& param, const series& ser, std::string_view separator)
{
    return fmt::format("{}{}{}", text_name(param, ser.front()), separator, text_name(param, ser.back()));

} // acmacs::time_series::v2::range_name

// ----------------------------------------------------------------------

std::pair<date::year_month_day, date::year_month_day> acmacs::time_series::v2::suggest_start_end(const parameters& param, const date_stat_t& stat)
{
    if (stat.empty())
        return {date::invalid_date(), date::invalid_date()};

    const auto distance = [&param](const date::year_month_day& d1, const date::year_month_day& d2) -> date::period_diff_t {
        switch (param.intervl) {
            case interval::year:
                return date::years_between_dates(d1, d2) / param.number_of_intervals;
            case interval::month:
                return date::months_between_dates(d1, d2) / param.number_of_intervals;
            case interval::week:
                return date::weeks_between_dates(d1, d2) / param.number_of_intervals;
            case interval::day:
                return date::days_between_dates(d1, d2) / param.number_of_intervals;
        }
        return 1; // hey g++-9
    };

    const auto next = [&param](date::year_month_day& date) -> date::year_month_day& {
        switch (param.intervl) {
            case interval::year:
                return date::increment_year(date, param.number_of_intervals);
            case interval::month:
                return date::increment_month(date, param.number_of_intervals);
            case interval::week:
                return date::increment_week(date, param.number_of_intervals);
            case interval::day:
                return date::increment_day(date, param.number_of_intervals);
        }
        return date; // hey g++-9
    };

    std::vector<std::pair<date::year_month_day, date::year_month_day>> chunks;
    auto it = std::begin(stat.counter());
    auto prev{it->first};
    auto cur = prev, start = prev;
    for (++it; it != std::end(stat.counter()); ++it) {
        cur = it->first;
        if (distance(prev, cur) > 2) {
            chunks.emplace_back(start, next(prev));
            start = cur;
        }
        prev = cur;
    }
    if (start != prev)
        chunks.emplace_back(start, next(prev));
    return *std::max_element(std::begin(chunks), std::end(chunks), [distance](const auto& e1, const auto& e2) { return distance(e1.first, e1.second) < distance(e2.first, e2.second); });

} // acmacs::time_series::v2::suggest_start_end

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
