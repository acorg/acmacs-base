#pragma once

#include <map>
#include <array>

#include "acmacs-base/stream.hh"

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

        void count(const Obj& aObj) { ++counter_[aObj]; }
        const auto& max() const { return *std::max_element(counter_.begin(), counter_.end(), [](const auto& e1, const auto& e2) { return e1.second < e2.second; }); }

        auto sorted_max_first() const
            {
                std::vector<std::reference_wrapper<value_type>> result(counter_.begin(), counter_.end());
                std::sort(result.begin(), result.end(), [](const auto& e1, const auto& e2) { return e1.second > e2.second; });
                return result;
            }

        constexpr const auto& counter() const { return counter_; }

        friend std::ostream& operator << (std::ostream& out, const Counter& counter)
            {
                return out << counter.counter_;
            }

     private:
        container_type counter_;

    }; // class Counter<Obj>

    template <typename Iter, typename F> Counter(Iter first, Iter last, F func) -> Counter<decltype(func(*first))>;

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
        template <typename Container, typename F> CounterChar(const Container& container, F func) : CounterChar(std::begin(container), std::end(container), func) {}
        void count(char aObj) { ++counter_[static_cast<unsigned char>(aObj)]; }

        std::pair<char, size_t> max() const
        {
            const auto me = std::max_element(counter_.begin(), counter_.end(), [](size_t e1, size_t e2) { return e1 < e2; });
            return {static_cast<char>(me - counter_.begin()), *me};
        }

      private:
        std::array<size_t, 256> counter_;
    };

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
