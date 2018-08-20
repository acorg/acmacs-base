#pragma once

#include <map>

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

        void add(const Obj& aObj) { ++counter_[aObj]; }

        const auto& max() const { return *std::max_element(counter_.begin(), counter_.end(), [](const auto& e1, const auto& e2) { return e1.second < e2.second; }); }

        auto sorted_max_first() const
            {
                std::vector<std::reference_wrapper<value_type>> result(counter_.begin(), counter_.end());
                std::sort(result.begin(), result.end(), [](const auto& e1, const auto& e2) { return e1.second > e2.second; });
                return result;
            }

        friend std::ostream& operator << (std::ostream& out, const Counter& counter)
            {
                return out << counter.counter_;
            }

     private:
        container_type counter_;

    }; // class Counter<Obj>

    template <typename Iter, typename F> Counter(Iter first, Iter last, F func) -> Counter<decltype(func(*first))>;

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
