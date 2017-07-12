#pragma once

#include <type_traits>
#include <iterator>

// ----------------------------------------------------------------------

  // void_t is a C++17 feature
template<class ...> using void_t = void; // http://stackoverflow.com/questions/26513095/void-t-can-implement-concepts

// ----------------------------------------------------------------------

namespace ad_sfinae
{

// ----------------------------------------------------------------------

    template <typename T, typename = void> struct castable_to_char : public std::false_type {};
    template <typename T> struct castable_to_char<T, void_t<decltype(static_cast<char>(std::declval<T>()))>> : public std::true_type {};

// ----------------------------------------------------------------------

// for container allowing std::begin(c) and std::end(c)
// https://stackoverflow.com/questions/30244447/how-to-sfinae-out-non-containers-parameters

    template <typename T, typename = void> struct is_std_container : std::false_type { };

    template <typename T> struct is_std_container<T,
                                                  void_t<decltype(std::begin(std::declval<T&>())),
                                                         decltype(std::end(std::declval<T&>())),
                                                         typename T::value_type
                                                         >>
        : std::true_type { };

// ----------------------------------------------------------------------

    // template <typename T, typename = void> struct is_list : std::false_type { };

    // template <typename T> struct is_list<T,
    //                                      void_t<decltype(std::begin(std::declval<T&>())),
    //                                             decltype(std::end(std::declval<T&>())),
    //                                             decltype(std::declval<T&>().operator[](static_cast<size_t>(0))),
    //                                             typename T::value_type
    //                                             >>
    //     : std::true_type { };

// ----------------------------------------------------------------------

} // namespace ad_sfinae

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
