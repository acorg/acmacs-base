#pragma once

#include <map>
#include <array>
#include <numeric>

#include "acmacs-base/stream.hh"
#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    template <typename Obj> class Counter
    {
     public:
        using container_type = std::map<Obj, size_t>;
        using value_type = typename container_type::value_type;

        Counter() = default;
        template <typename Iter, typename F> Counter(Iter first, Iter last, F func)
            {
                for (; first != last; ++first)
                    ++counter_[func(*first)];
            }
        template <typename Container, typename F> Counter(const Container& container, F func) : Counter(std::begin(container), std::end(container), func) {}

        template <typename S> void count(const S& aObj) { ++counter_[Obj{aObj}]; }
        template <typename S> void count_if(bool cond, const S& aObj) { if (cond) ++counter_[Obj{aObj}]; }

        const auto& max() const { return *std::max_element(counter_.begin(), counter_.end(), [](const auto& e1, const auto& e2) { return e1.second < e2.second; }); }

        auto sorted_max_first() const
            {
                std::vector<const value_type*> result(counter_.size());
                std::transform(std::begin(counter_), std::end(counter_), std::begin(result), [](const auto& ee) { return &ee; });
                std::sort(result.begin(), result.end(), [](const auto& e1, const auto& e2) { return e1->second > e2->second; });
                return result;
            }

        constexpr const auto& counter() const { return counter_; }
        constexpr size_t size() const { return counter_.size(); }
        constexpr bool empty() const { return counter_.empty(); }

        template <typename S> size_t operator[](const S& look_for) const
        {
            if (const auto found = counter_.find(look_for); found != counter_.end())
                return found->second;
            else
                return 0;
        }

        std::string report(std::string_view format) const
        {
            fmt::memory_buffer out;
            for (const auto& entry : counter_)
                format_entry(out, format, entry);
            return fmt::to_string(out);
        }

        std::string report() const { return report("{value}  {counter:6d}\n"); }

        std::string report_sorted_max_first(std::string_view format) const
        {
            fmt::memory_buffer out;
            for (const auto& entry : sorted_max_first())
                format_entry(out, format, *entry);
            return fmt::to_string(out);
        }

        std::string report_sorted_max_first() const { return report_sorted_max_first("{counter:6d} {value}\n"); }

        size_t total() const
        {
            return std::accumulate(std::begin(counter_), std::end(counter_), 0UL, [](size_t sum, const auto& en) { return sum + en.second; });
        }

     private:
        container_type counter_;

        void format_entry(fmt::memory_buffer& out, std::string_view format, const value_type& entry) const
        {
                fmt::format_to(out, format, fmt::arg("quoted", fmt::format("\"{}\"", entry.first)), fmt::arg("value", entry.first), fmt::arg("counter", entry.second),
                               fmt::arg("first", entry.first), fmt::arg("quoted_first", fmt::format("\"{}\"", entry.first)), fmt::arg("second", entry.second));
        }

    }; // class Counter<Obj>

    template <typename Iter, typename F> Counter(Iter first, Iter last, F func) -> Counter<decltype(func(*first))>;
    template <typename Container, typename F> Counter(const Container& cont, F func) -> Counter<decltype(func(*std::begin(cont)))>;

    class CounterChar
    {
      public:
        CounterChar() { std::fill(std::begin(counter_), std::end(counter_), 0UL); }
        template <typename Iter, typename F> CounterChar(Iter first, Iter last, F func)
            : CounterChar()
        {
            for (; first != last; ++first)
                ++counter_[func(*first)];
        }
        template <typename Iter> CounterChar(Iter first, Iter last)
            : CounterChar()
        {
            for (; first != last; ++first)
                ++counter_[static_cast<size_t>(*first)];
        }
        template <typename Container, typename F> CounterChar(const Container& container, F func) : CounterChar(std::begin(container), std::end(container), func) {}
        template <typename Container> CounterChar(const Container& container) : CounterChar(std::begin(container), std::end(container)) {}

        void count(char aObj) { ++counter_[static_cast<unsigned char>(aObj)]; }

        bool empty() const { return std::all_of(std::begin(counter_), std::end(counter_), [](size_t val) { return val == 0; }); }
        size_t size() const { return static_cast<size_t>(std::count_if(std::begin(counter_), std::end(counter_), [](size_t val) { return val > 0UL; })); }
        size_t total() const { return std::accumulate(std::begin(counter_), std::end(counter_), 0UL, [](size_t sum, size_t val) { return sum + val; }); }

        std::pair<char, size_t> max() const
        {
            const auto me = std::max_element(counter_.begin(), counter_.end(), [](size_t e1, size_t e2) { return e1 < e2; });
            return {static_cast<char>(me - counter_.begin()), *me};
        }

        std::vector<char> sorted() const
        {
            std::vector<char> res;
            for (auto it = std::begin(counter_); it != std::end(counter_); ++it) {
                if (*it > 0)
                    res.push_back(static_cast<char>(it - std::begin(counter_)));
            }
            std::sort(std::begin(res), std::end(res), [this](auto e1, auto e2) { return counter_[static_cast<size_t>(e1)] > counter_[static_cast<size_t>(e2)]; });
            return res;
        }

        enum class sorted { no, yes };
        std::vector<std::pair<char, size_t>> pairs(enum sorted srt) const
        {
            std::vector<std::pair<char, size_t>> result;
            for (auto it = std::begin(counter_); it != std::end(counter_); ++it) {
                if (*it > 0)
                    result.emplace_back(static_cast<char>(it - std::begin(counter_)), *it);
            }
            if (srt == sorted::yes)
                std::sort(std::begin(result), std::end(result), [](const auto& e1, const auto& e2) { return e1.second > e2.second; });
            return result;
        }

        std::string report(std::string_view format) const
        {
            fmt::memory_buffer out;
            for (const auto& entry : pairs(sorted::no))
                format_entry(out, format, entry);
            return fmt::to_string(out);
        }

        std::string report() const { return report_sorted_max_first("{value} {counter:6d}\n"); }

        std::string report_sorted_max_first(std::string_view format) const
        {
            fmt::memory_buffer out;
            for (const auto& entry : pairs(sorted::yes))
                format_entry(out, format, entry);
            return fmt::to_string(out);
        }

        std::string report_sorted_max_first() const { return report_sorted_max_first("{counter:6d} {value}\n"); }

        size_t operator[](char val) const { return counter_[static_cast<size_t>(val)]; }

        constexpr const auto& counter() const { return counter_; }

      private:
        std::array<size_t, 256> counter_;

        void format_entry(fmt::memory_buffer& out, std::string_view format, const std::pair<char, size_t>& entry) const
        {
                fmt::format_to(out, format, fmt::arg("value", entry.first), fmt::arg("counter", entry.second),
                               fmt::arg("first", entry.first), fmt::arg("quoted_first", fmt::format("\"{}\"", entry.first)), fmt::arg("second", entry.second));
        }

    };

} // namespace acmacs

// ----------------------------------------------------------------------

template <typename Key> struct fmt::formatter<acmacs::Counter<Key>> : public fmt::formatter<acmacs::fmt_default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::Counter<Key>& counter, FormatContext& ctx)
    {
        return format_to(ctx.out(), "counter{{{}}}", counter.counter());
    }
};

template <> struct fmt::formatter<acmacs::CounterChar> : public fmt::formatter<acmacs::fmt_default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::CounterChar& counter, FormatContext& ctx)
    {
        auto out = format_to(ctx.out(), "counter{{");
        const auto keys = counter.sorted();
        for (auto it = keys.begin(); it != keys.end(); ++it) {
            if (it != keys.begin())
                out = format_to(out, ", ");
            out = format_to(out, "{}: {}", *it, counter[*it]);
        }
        return format_to(out, "}}");
    }
};


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
