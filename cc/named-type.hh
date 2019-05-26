#pragma once

#include <iostream>

#include "acmacs-base/string.hh"

// Simplified version of https://github.com/joboccara/NamedType
// Also see https://github.com/Enhex/phantom_type

// ----------------------------------------------------------------------

#include <type_traits>

namespace acmacs
{
    template <typename T, typename Tag> class named_t
    {
      public:
        using value_type = T;
        template <typename TT> using is_not_reference_t = typename std::enable_if<!std::is_reference<TT>::value, void>::type;

        explicit constexpr named_t() = default;
        template <typename T2> explicit constexpr named_t(T2&& value) : value_(std::forward<T2>(value)) {}

        // explicit constexpr named_t(const T& value) : value_(value) {}
        // explicit constexpr named_t(T&& value) : value_(std::move(value)) {}
        // template <typename T2 = T, typename = is_not_reference_t<T2>> explicit constexpr named_t(T2&& value) : value_(std::move(value)) {}

        constexpr T& get() noexcept { return value_; }
        constexpr T const& get() const noexcept { return value_; }
        constexpr T& operator*() noexcept { return value_; }
        constexpr T const& operator*() const noexcept { return value_; }
        constexpr const T* operator->() const noexcept { return &value_; }
        explicit constexpr operator T&() noexcept { return value_; }
        explicit constexpr operator const T&() const noexcept { return value_; }

        template <typename T1, typename Tag1> constexpr bool operator==(const named_t<T1, Tag1>& rhs) const noexcept { return get() == rhs.get(); }
        template <typename T1, typename Tag1> constexpr bool operator!=(const named_t<T1, Tag1>& rhs) const noexcept { return !operator==(rhs); }
        template <typename T1, typename Tag1> constexpr bool operator<(const named_t<T1, Tag1>& rhs) const noexcept { return get() < rhs.get(); }

      private:
        T value_;
    };

    template <typename T, typename Tag> inline std::ostream& operator<<(std::ostream& out, const named_t<T, Tag>& named) { return out << named.get(); }
    template <typename T, typename Tag> inline std::string to_string(const named_t<T, Tag>& named) { return acmacs::to_string(named.get()); }

} // namespace acmacs


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
