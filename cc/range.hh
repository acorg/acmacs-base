#pragma once

#include <iterator>
#include <stdexcept>
#include <limits>

// ----------------------------------------------------------------------

template <typename T> class Range
{
 private:
    class iterator : public std::iterator<std::input_iterator_tag, T>
    {
     public:
        inline iterator() : current(std::numeric_limits<T>::max()), step(0), last(std::numeric_limits<T>::max()) {}
        inline iterator(T aFirst, T aLast, T aStep) : current(aFirst), step(aStep), last(aLast) { if (end()) throw std::runtime_error("Invalid range with step 0"); }

        inline iterator& operator++() { current += step; if (current >= last) step = T{0}; return *this;}
        inline iterator operator++(int) { iterator result = *this; ++(*this); return result; }
        inline bool operator==(const iterator& other) const { return (end() && other.end()) || current == other.current; }
        inline bool operator!=(const iterator& other) const { return !(*this == other); }
        inline T operator*() const { return current; }

     private:
        T current;
        T step;
        T last;

        inline bool end() const { return step == T{0}; }
    };

 public:
    static inline iterator begin(T first, T last, T step = T{1})
        {
            return {first, last, step};
        }

    static inline iterator begin(T last)
        {
            return {T{0}, last, T{1}};
        }

    static inline iterator end()
        {
            return {};
        }

}; // class Range

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
