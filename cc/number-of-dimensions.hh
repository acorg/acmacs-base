#pragma once

#include "acmacs-base/named-type.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    using number_of_dimensions_t = named_size_t<struct number_of_dimensions_tag>;

    constexpr bool valid(number_of_dimensions_t nd) { return nd.get() > 0; }

    // class number_of_dimensions_t
    // {
    //   public:
    //     explicit constexpr number_of_dimensions_t(size_t value) : value_{value} {}
    //     // explicit constexpr number_of_dimensions_t(int value) : value_{static_cast<size_t>(value)} {}

    //     constexpr size_t& get() noexcept { return value_; }
    //     constexpr size_t get() const noexcept { return value_; }
    //     constexpr size_t& operator*() noexcept { return value_; }
    //     constexpr size_t operator*() const noexcept { return value_; }
    //     explicit constexpr operator size_t&() noexcept { return value_; }
    //     explicit constexpr operator size_t() const noexcept { return value_; }

    //     constexpr number_of_dimensions_t& operator++() { ++value_; return *this; }
    //     constexpr bool operator==(number_of_dimensions_t rhs) const noexcept { return get() == rhs.get(); }
    //     constexpr bool operator!=(number_of_dimensions_t rhs) const noexcept { return !operator==(rhs); }
    //     constexpr bool operator< (number_of_dimensions_t rhs) const noexcept { return get() < rhs.get(); }
    //     constexpr bool operator<=(number_of_dimensions_t rhs) const noexcept { return get() <= rhs.get(); }
    //     constexpr bool operator> (number_of_dimensions_t rhs) const noexcept { return get() >  rhs.get(); }
    //     constexpr bool operator>=(number_of_dimensions_t rhs) const noexcept { return get() >= rhs.get(); }

    //     constexpr bool valid() const noexcept { return value_ > 0; }

    //   private:
    //     size_t value_;

    // }; // class number_of_dimensions_t

    // inline std::ostream& operator<<(std::ostream& out, number_of_dimensions_t nd) { return out << nd.get(); }
    // inline std::string to_string(number_of_dimensions_t nd) { return acmacs::to_string(nd.get()); }

    // index_iterator(number_of_dimensions_t) -> index_iterator<number_of_dimensions_t>;

    // range(int, number_of_dimensions_t) -> range<number_of_dimensions_t>;
    // range(size_t, number_of_dimensions_t) -> range<number_of_dimensions_t>;
    // range(number_of_dimensions_t) -> range<number_of_dimensions_t>;
    // range(number_of_dimensions_t, number_of_dimensions_t) -> range<number_of_dimensions_t>;

}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
