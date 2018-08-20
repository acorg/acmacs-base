#pragma once

#include <map>

#include "acmacs-base/stream.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    template <typename Obj> class Counter
    {
     public:
        Counter() = default;
        template <typename Iter, typename F> Counter(Iter first, Iter last, F func)
            {
                for (; first != last; ++first)
                    ++counter_[func(*first)];
            }

        void add(const Obj& aObj) { ++counter_[aObj]; }

        const auto& max() const { return *std::max_element(counter_.begin(), counter_.end(), [](const auto& e1, const auto& e2) { return e1.second < e2.second; }); }

        friend std::ostream& operator << (std::ostream& out, const Counter& counter)
            {
                return out << counter.counter_;
            }

     private:
        std::map<Obj, size_t> counter_;

    }; // class Counter<Obj>

    template <typename Iter, typename F> Counter(Iter first, Iter last, F func) -> Counter<decltype(func(*first))>;

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
