#pragma once

#include <iterator>
#include <stdexcept>
#include <limits>
#include <functional>

#include "acmacs-base/stream.hh"

// ----------------------------------------------------------------------

namespace _acmacs_base_internal
{
    template <typename T, typename Increment> class input_iterator : public std::iterator<std::input_iterator_tag, T>
    {
     public:
        constexpr static const T End{std::numeric_limits<T>::max()};

        inline input_iterator() : current(End) {}
        inline input_iterator(T aFirst, const Increment& aIncrement) : current(aFirst), increment(aIncrement) { increment.find_valid(current); }
        inline input_iterator(T aFirst, Increment&& aIncrement) : current(aFirst), increment(std::move(aIncrement)) { increment.find_valid(current); }

        inline input_iterator& operator++() { increment(current); return *this;}
          // inline input_iterator operator++(int) { input_iterator result = *this; ++(*this); return result; }
        inline bool operator==(const input_iterator& other) const { return current == other.current; }
        inline bool operator!=(const input_iterator& other) const { return !(*this == other); }
        inline const T& operator*() const { return current; }
        inline T& operator*() { return current; }

     private:
        T current;
        Increment increment;

    }; // input_iterator<>

} // namespace _acmacs_base_internal

// ----------------------------------------------------------------------

template <typename T> class Range
{
 private:
    class increment
    {
     public:
        inline increment() {}
        inline increment(T aStep, T aLast) : step(aStep), last(aLast) { if (step == T{0}) throw std::runtime_error("Invalid range with step 0"); }
        inline void operator()(T& current)
            {
                if (current != _acmacs_base_internal::input_iterator<T, increment>::End) {
                    current += step;
                    if (step > 0) {
                        if (current >= last)
                            current = _acmacs_base_internal::input_iterator<T, increment>::End;
                    }
                    else {
                        if (current <= last)
                            current = _acmacs_base_internal::input_iterator<T, increment>::End;
                    }
                }
            }

        inline void find_valid(T&) {}

     private:
        T step;
        T last;
    };

    class iterator : public _acmacs_base_internal::input_iterator<T, increment>
    {
     public:
        inline iterator() : _acmacs_base_internal::input_iterator<T, increment>() {}
        inline iterator(T aFirst, T aLast, T aStep) : _acmacs_base_internal::input_iterator<T, increment>(aFirst, {aStep, aLast}) {}
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

class IndexGenerator
{
 public:
    using Validator = std::function<bool (size_t)>;

 private:
    class increment
    {
     public:
        inline increment() {}
        inline increment(size_t aLast, Validator aValidator) : last(aLast), valid(aValidator) {}
        inline bool end(size_t current) const { return current == _acmacs_base_internal::input_iterator<size_t, increment>::End; }
        inline size_t end() const { return _acmacs_base_internal::input_iterator<size_t, increment>::End; }

        inline void operator()(size_t& current)
            {
                if (!end(current)) {
                    ++current;
                    while (current < last && !valid(current))
                        ++current;
                    if (current >= last)
                        current = end();
                }
            }

        inline void find_valid(size_t& current)
            {
                while (!end(current) && !valid(current))
                    operator()(current);
            }

        size_t last;
        Validator valid;
    };

 public:
    inline IndexGenerator(size_t aFirst, size_t aLast, Validator aValidator) : mFirst(aFirst), mIncrement(aLast, aValidator) {}
    inline IndexGenerator(size_t aLast, Validator aValidator) : mFirst(0), mIncrement(aLast, aValidator) {}

    using iterator = _acmacs_base_internal::input_iterator<size_t, IndexGenerator::increment>;

    inline iterator begin() const { return {mFirst, mIncrement}; }
    inline iterator end() const { return {}; }

 private:
    size_t mFirst;
    increment mIncrement;

}; // class IndexGenerator

inline std::ostream& operator << (std::ostream& out, const IndexGenerator& aGen)
{
    return stream_internal::write_to_stream(out, aGen, "<", ">", ", ");
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End: