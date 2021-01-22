#pragma once

#include <iterator>

// ======================================================================

namespace acmacs
{
    template <typename Parent, typename Reference, typename Index = size_t> class iterator
    {
      public:
        using reference = Reference;
        using pointer = typename std::add_pointer<Reference>::type;
        using value_type = typename std::remove_reference<Reference>::type;
        using difference_type = ssize_t;
        using iterator_category = std::random_access_iterator_tag;

        constexpr iterator& operator++()
        {
            ++mIndex;
            return *this;
        }
        constexpr iterator& operator+=(difference_type n)
        {
            mIndex += n;
            return *this;
        }
        constexpr iterator& operator-=(difference_type n)
        {
            mIndex -= n;
            return *this;
        }
        constexpr iterator operator-(difference_type n)
        {
            iterator temp = *this;
            return temp -= n;
        }
        constexpr difference_type operator-(const iterator& rhs) { return mIndex - rhs.mIndex; }
        constexpr bool operator==(const iterator& other) const { return &mParent == &other.mParent && mIndex == other.mIndex; }
        constexpr bool operator!=(const iterator& other) const { return &mParent != &other.mParent || mIndex != other.mIndex; }
        constexpr reference operator*() { return mParent[mIndex]; }
        constexpr Index index() const { return mIndex; }
        constexpr bool operator<(const iterator& rhs) const { return mIndex < rhs.mIndex; }
        constexpr bool operator<=(const iterator& rhs) const { return mIndex <= rhs.mIndex; }
        constexpr bool operator>(const iterator& rhs) const { return mIndex > rhs.mIndex; }
        constexpr bool operator>=(const iterator& rhs) const { return mIndex >= rhs.mIndex; }

      private:
        iterator(const Parent& aParent, Index aIndex) : mParent{aParent}, mIndex{aIndex} {}

        const Parent& mParent;
        Index mIndex;

        friend Parent;
    };

} // namespace acmacs

// ======================================================================

// ----------------------------------------------------------------------
// polyfill for std::ostream_joiner of c++17

// #if __cplusplus <= 201500

// clang 8.1 on macOS 10.12
// namespace polyfill
// {
//     template <typename Stream, typename _DelimT/* , typename _CharT = char, typename _Traits = char_traits<_CharT> */> class ostream_joiner
//     {
//      public:
//         using char_type = typename Stream::char_type; // _CharT;
//         using traits_type = typename Stream::traits_type; //_Traits;
//         using iterator_category = std::output_iterator_tag;

//         using value_type = void;
//         using difference_type = void;
//         using pointer = void;
//         using reference = void;

//         inline ostream_joiner(Stream& __os, const _DelimT& __delimiter)
//               // noexcept(is_nothrow_copy_constructible_v<_DelimT>)
//             : _M_out(std::addressof(__os)), _M_delim(__delimiter)
//             { }

//         inline ostream_joiner(Stream& __os, _DelimT&& __delimiter)
//               // noexcept(is_nothrow_move_constructible_v<_DelimT>)
//             : _M_out(std::addressof(__os)), _M_delim(std::move(__delimiter))
//             { }

//         template <typename _Tp> inline ostream_joiner& operator=(const _Tp& __value)
//             {
//                 if (!_M_first)
//                     *_M_out << _M_delim;
//                 _M_first = false;
//                 *_M_out << __value;
//                 return *this;
//             }

//         ostream_joiner& operator*() noexcept { return *this; }
//         ostream_joiner& operator++() noexcept { return *this; }
//         ostream_joiner& operator++(int) noexcept { return *this; }

//      private:
//         Stream* _M_out;
//         _DelimT _M_delim;
//         bool _M_first = true;
//     };

//     template <typename Stream, typename _DelimT/* , typename _CharT = char, typename _Traits = char_traits<_CharT> */> inline ostream_joiner<Stream, std::decay_t<_DelimT>> make_ostream_joiner(Stream& __os, _DelimT&& __delimiter)
//     {
//         return { __os, std::forward<_DelimT>(__delimiter) };
//     }

// } // namespace polyfill

// #else
// // gcc 6.2+
// #include <experimental/iterator>
// namespace std
// {
//     template<typename _DelimT> using ostream_joiner = experimental::fundamentals_v2::ostream_joiner<_DelimT>;
// }
// #endif

// ======================================================================
// https://internalpointers.com/post/writing-custom-iterators-modern-cpp
// ======================================================================
//
// #include <iterator>
// #include <cstddef>
//
// class Integers
// {
// public:
//     struct Iterator
//     {
//         using iterator_category = std::forward_iterator_tag;
//         using difference_type   = std::ptrdiff_t;
//         using value_type        = int;
//         using pointer           = int*;
//         using reference         = int&;
//
//         Iterator(pointer ptr) : m_ptr(ptr) {}
//
//         reference operator*() const { return *m_ptr; }
//         pointer operator->() { return m_ptr; }
//         Iterator& operator++() { m_ptr++; return *this; }
//         Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
//         friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
//         friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };
//
//     private:
//         pointer m_ptr;
//     };
//
//     Iterator begin() { return Iterator(&m_data[0]); }
//     Iterator end()   { return Iterator(&m_data[200]); }
//
// private:
//     int m_data[200];
// };

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
