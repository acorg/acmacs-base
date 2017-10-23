#pragma once

// https://infektor.net/posts/2017-03-31-range-based-enumerate.html

#include <iterator>
#include <utility>

// ----------------------------------------------------------------------

namespace _enumerate_internal
{
    template <typename Iterator, typename index_type> struct enumerate_iterator
    {
        using iterator = Iterator;
          //using index_type = typename std::iterator_traits<iterator>::difference_type;
        using reference = typename std::iterator_traits<iterator>::reference;

        enumerate_iterator(index_type aIndex, iterator aIterator) : index(aIndex), iter(aIterator) {}
        enumerate_iterator& operator++() { ++index; ++iter; return *this; }
        bool operator!=(const enumerate_iterator &other) const { return iter != other.iter; }
        std::pair<index_type&, reference> operator*() { return {index, *iter}; }

     private:
        index_type index;
        iterator iter;
    };

      // ----------------------------------------------------------------------

    template <typename Iterator, typename index_type> struct enumerate_range
    {
        // using index_type = typename std::iterator_traits<Iterator>::difference_type;
        using iterator = enumerate_iterator<Iterator, index_type>;

        enumerate_range(Iterator aFirst, Iterator aLast, index_type aInitial) : first(aFirst), last(aLast), initial(aInitial) {}
        iterator begin() const { return iterator(initial, first); }
        iterator end() const { return iterator(0, last); }

     private:
        Iterator first;
        Iterator last;
        index_type initial;
    };

} // namespace _enumerate_internal

// ----------------------------------------------------------------------

namespace acmacs
{
    template <typename Iterator, typename index_type> auto enumerate(Iterator first, Iterator last, index_type initial = 0)
    {
        return _enumerate_internal::enumerate_range<Iterator, index_type>(first, last, initial);
    }

    template <typename Container> auto enumerate(Container& content)
    {
        using iter_type = decltype(std::begin(content));
        return _enumerate_internal::enumerate_range<iter_type, size_t>(std::begin(content), std::end(content), 0U);
    }

    template <typename Container> auto enumerate(const Container& content)
    {
        using iter_type = decltype(std::begin(content));
        return _enumerate_internal::enumerate_range<iter_type, size_t>(std::begin(content), std::end(content), 0U);
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
