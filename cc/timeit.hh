#pragma once

#include <chrono>

#include "acmacs-base/log.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    using clock_t = std::chrono::high_resolution_clock;
    using timestamp_t = decltype(clock_t::now());
    using duration_t = std::chrono::nanoseconds;

    inline timestamp_t timestamp() { return clock_t::now(); }
    inline duration_t elapsed(timestamp_t start) { return std::chrono::duration_cast<duration_t>(timestamp() - start); }
    std::string format_duration(duration_t duration);
    inline double elapsed_seconds(timestamp_t start) { return std::chrono::duration<double>{timestamp() - start}.count(); }

} // namespace acmacs

// ----------------------------------------------------------------------

enum class report_time { no, yes };

constexpr report_time do_report_time(bool do_report) { return do_report ? report_time::yes : report_time::no; }

// ----------------------------------------------------------------------

class Timeit
{
  public:
    Timeit(std::string_view msg, report_time aReport = report_time::yes, const acmacs::log::source_location& sl = acmacs::log::source_location{})
        : message_{msg}, report_(aReport), start_{acmacs::timestamp()}, sl_{sl}
    {
    }
    ~Timeit() { report(); }

    void report() const
    {
        acmacs::log::print(sl_, report_ == report_time::yes, acmacs::log::prefix::info, "{}: {}", message_, acmacs::format_duration(acmacs::elapsed(start_)));
        report_ = report_time::no;
    }

    void message_append(const std::string& to_append) { message_ += to_append; }

  private:
    std::string message_;
    mutable report_time report_;
    acmacs::timestamp_t start_;
    const acmacs::log::source_location sl_;
};

// ----------------------------------------------------------------------

template <typename Func> auto timeit(std::string_view msg, Func&& func, const acmacs::log::source_location& sl = acmacs::log::source_location{})
    requires std::is_invocable_v<Func>
{
    const auto start{acmacs::timestamp()};
    const auto print = [&sl, msg, start]() { acmacs::log::print(sl, true, acmacs::log::prefix::info, "{}: {}", msg, acmacs::format_duration(acmacs::elapsed(start))); };

    if constexpr (std::is_same_v<std::invoke_result_t<Func>, void>) {
        func();
        print();
    }
    else {
        const std::invoke_result_t<Func> result = func();
        print();
        return result;
    }
}

// ----------------------------------------------------------------------

template <typename Func> auto timeit(std::string_view pre, std::string_view after, Func&& func, const acmacs::log::source_location& sl = acmacs::log::source_location{})
    requires std::is_invocable_v<Func>
{
    acmacs::log::print(sl, true, acmacs::log::prefix::info, "{}", pre);
    return timeit(after, std::forward<Func>(func), sl);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
