#pragma once

#include <type_traits>
// #include <iterator>

// ----------------------------------------------------------------------

namespace acmacs::sfinae
{
    inline namespace v1
    {
        template <typename T, typename = void, typename = void> struct container_with_iterator : std::false_type
        {
        };
        // template <typename T> struct container_with_iterator<T, std::void_t<decltype(std::declval<T>().begin())>> : std::true_type
        // {
        // };
        template <typename T> struct container_with_iterator<T, decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())> : std::true_type
        {
        };

        template <typename T, typename = void> struct container_with_resize : std::false_type
        {
        };
        template <typename T> struct container_with_resize<T, decltype(std::declval<T>().resize(1))> : std::true_type
        {
        };

          // ----------------------------------------------------------------------

        template <typename T> struct is_string : std::false_type
        {
        };
        template <> struct is_string<const char*> : std::true_type
        {
        };
        template <> struct is_string<std::string> : std::true_type
        {
        };
        template <> struct is_string<std::string_view> : std::true_type
        {
        };

        template <typename T> inline constexpr bool is_string_v = is_string<T>::value;

          // ----------------------------------------------------------------------

    } // namespace v1
} // namespace acmacs::sfinae

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
