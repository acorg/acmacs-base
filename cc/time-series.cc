#include "acmacs-base/time-series.hh"
#include "acmacs-base/rjson-v2.hh"
#include "acmacs-base/enumerate.hh"

// ----------------------------------------------------------------------

inline date::year_month_day next(const date::year_month_day& current, acmacs::time_series::v2::interval intervl)
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

} // next

// ----------------------------------------------------------------------

inline date::year_month_day first(const date::year_month_day& current, acmacs::time_series::v2::interval intervl)
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

} // next

// ----------------------------------------------------------------------

acmacs::time_series::v2::series acmacs::time_series::v2::make(const parameters& param)
{
    const auto increment = [](const date::year_month_day& cur, interval intervl, size_t count) {
        auto result = cur;
        for (size_t i = 0; i < count; ++i)
            result = next(result, intervl);
        return result;
    };

    series result;
    for (auto current = first(param.first, param.intervl); current < param.after_last; ) {
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
    if (interval_name == "monthly"sv)
        return acmacs::time_series::interval::month;
    else if (interval_name == "yearly"sv)
        return acmacs::time_series::interval::year;
    else if (interval_name == "weekly"sv)
        return acmacs::time_series::interval::week;
    else if (interval_name == "daily"sv)
        return acmacs::time_series::interval::day;
    else {
        fmt::print(stderr, "WARNING: unrecognized interval specification: \"{}\", month assumed\n", interval_name);
        return acmacs::time_series::interval::month;
    }

} // acmacs::time_series::v2::interval_from_string

// ----------------------------------------------------------------------

acmacs::time_series::v2::parameters& acmacs::time_series::v2::update(const rjson::v2::value& source, parameters& param)
{
    using namespace std::string_view_literals;
    if (const auto& start = source["start"sv]; !start.is_null())
        param.first = date::from_string(start.to<std::string_view>(), date::allow_incomplete::yes);
    if (const auto& end = source["end"sv]; !end.is_null())
        param.after_last = date::from_string(end.to<std::string_view>(), date::allow_incomplete::yes);
    std::visit(
        [&param]<typename T>(T && arg) {
            if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
                param.intervl = interval_from_string(arg);
            }
            else if constexpr (std::is_same_v<std::decay_t<T>, rjson::v2::object>) {
                if (const auto& mon = arg.get("month"sv); !mon.is_null()) {
                    param.intervl = interval::month;
                    param.number_of_intervals = mon.template to<size_t>();
                }
                else if (const auto& yea = arg.get("year"sv); !yea.is_null()) {
                    param.intervl = interval::year;
                    param.number_of_intervals = yea.template to<size_t>();
                }
                else if (const auto& wee = arg.get("week"sv); !wee.is_null()) {
                    param.intervl = interval::week;
                    param.number_of_intervals = wee.template to<size_t>();
                }
                else if (const auto& da = arg.get("day"sv); !da.is_null()) {
                    param.intervl = interval::day;
                    param.number_of_intervals = da.template to<size_t>();
                }
                else {
                    fmt::print(stderr, "WARNING: unrecognized interval specification: {}\n", arg);
                }
            }
            else if constexpr (!std::is_same_v<std::decay_t<T>, rjson::v2::const_null> && !std::is_same_v<std::decay_t<T>, rjson::v2::null>) {
                fmt::print(stderr, "WARNING: unrecognized interval specification: {}\n", arg);
            }
        },
        source["interval"].val_());
    return param;

} // acmacs::time_series::v2::make

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

std::string acmacs::time_series::v2::text_name(const slot& a_slot)
{
    if (const auto days = date::days_between_dates(a_slot.first, a_slot.after_last); days < 28)
        return date::display(a_slot.first);
    else if (days < 360)
        return date::monthtext_year(a_slot.first);
    else
        return date::year_4(a_slot.first);

} // acmacs::time_series::v2::text_name

// ----------------------------------------------------------------------

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
