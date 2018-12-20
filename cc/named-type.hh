#pragma once

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

        explicit constexpr named_t(const T& value) : value_(value) {}
        template <typename T_ = T, typename = is_not_reference_t<T_>> explicit constexpr named_t(T&& value) : value_(std::move(value)) {}

        constexpr T& get() { return value_; }
        constexpr T const& get() const { return value_; }
        explicit constexpr operator T&() noexcept { return value_; }
        explicit constexpr operator const T&() const noexcept { return value_; }

      private:
        T value_;
    };
    
} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
