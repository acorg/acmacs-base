#pragma once

#include <iterator>
#include <stdexcept>
#include <limits>
#include <functional>
#include <numeric>

#include "acmacs-base/throw.hh"
#include "acmacs-base/stream.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    namespace internal
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

    } // namespace internal

// ----------------------------------------------------------------------

// template <typename T> class Range
// {
//  private:
//     class increment
//     {
//      public:
//         inline increment() {}
//         inline increment(T aStep, T aLast) : step(aStep), last(aLast) { if (step == T{0}) THROW_OR_VOID(std::runtime_error("Invalid range with step 0")); }
//         inline void operator()(T& current)
//             {
//                 if (current != internal::input_iterator<T, increment>::End) {
//                     current += step;
//                     if (step > 0) {
//                         if (current >= last)
//                             current = internal::input_iterator<T, increment>::End;
//                     }
//                     else {
//                         if (current <= last)
//                             current = internal::input_iterator<T, increment>::End;
//                     }
//                 }
//             }

//         inline void find_valid(T&) {}

//      private:
//         T step;
//         T last;
//     };

//     class iterator : public internal::input_iterator<T, increment>
//     {
//      public:
//         inline iterator() : internal::input_iterator<T, increment>() {}
//         inline iterator(T aFirst, T aLast, T aStep) : internal::input_iterator<T, increment>(aFirst, {aStep, aLast}) {}
//     };

//  public:
//     static inline iterator begin(T first, T last, T step = T{1})
//         {
//             return {first, last, step};
//         }

//     static inline iterator begin(T last)
//         {
//             return {T{0}, last, T{1}};
//         }

//     static inline iterator end()
//         {
//             return {};
//         }

// }; // class Range

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
            inline bool end(size_t current) const { return current == internal::input_iterator<size_t, increment>::End; }
            inline size_t end() const { return internal::input_iterator<size_t, increment>::End; }

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

        using iterator = internal::input_iterator<size_t, IndexGenerator::increment>;

        inline iterator begin() const { return {mFirst, mIncrement}; }
        inline iterator end() const { return {}; }

     private:
        size_t mFirst;
        increment mIncrement;

    }; // class IndexGenerator

// ----------------------------------------------------------------------

    template <typename Index> class index_iterator
    {
     public:
        using difference_type = long;
        using value_type = Index;
        using pointer = Index*;
        using reference = Index&;
        using iterator_category = std::random_access_iterator_tag;

        inline index_iterator(value_type aValue) : value(aValue) {}
        inline index_iterator& operator++() { ++value; return *this;}
          //inline index_iterator operator++(int) { iterator retval = *this; ++(*this); return retval;}
        inline bool operator==(const index_iterator<Index>& other) const { return value == other.value; }
        inline bool operator!=(const index_iterator<Index>& other) const { return !(*this == other); }
        inline value_type operator*() const { return value; }
        inline difference_type operator-(const index_iterator<Index>& other) const { return static_cast<difference_type>(value) - static_cast<difference_type>(other.value); }

     private:
        value_type value;

    }; // class index_iterator<>

    index_iterator(size_t) -> index_iterator<size_t>;
    index_iterator(int) -> index_iterator<int>;

    template <typename Index> class incrementer
    {
     public:
        inline incrementer(Index aBegin, Index aEnd) : mBegin{aBegin}, mEnd{aEnd} {}

        inline index_iterator<Index> begin() { return mBegin; }
        inline index_iterator<Index> end() { return mEnd; }

     private:
        Index mBegin, mEnd;

    }; // class incrementer<>

    incrementer(size_t, size_t) -> incrementer<size_t>;
    incrementer(int, int) -> incrementer<int>;

// ----------------------------------------------------------------------

    template <typename Index> inline void fill_with_indexes(std::vector<Index>& aToFill, Index aBegin, Index aEnd)
    {
        aToFill.resize(aEnd - aBegin);
        std::iota(aToFill.begin(), aToFill.end(), aBegin);
    }

    template <typename Index> inline void fill_with_indexes(std::vector<Index>& aToFill, Index aSize)
    {
        aToFill.resize(aSize);
        std::iota(aToFill.begin(), aToFill.end(), 0);
    }

    template <typename Index> inline std::vector<Index> filled_with_indexes(Index aSize)
    {
        std::vector<Index> result(aSize);
        std::iota(result.begin(), result.end(), 0);
        return result;
    }

// ----------------------------------------------------------------------

} // namespace acmacs

// ----------------------------------------------------------------------


#ifdef ACMACS_TARGET_OS
inline std::ostream& operator << (std::ostream& out, const acmacs::IndexGenerator& aGen)
{
    return stream_internal::write_to_stream(out, aGen, "<", ">", ", ");
}
#endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
