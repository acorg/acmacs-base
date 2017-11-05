#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <typeinfo>

// ----------------------------------------------------------------------

#include "acmacs-base/to-string.hh"
#include "acmacs-base/stream.hh" // for debugging

// ----------------------------------------------------------------------

namespace to_json
{
    class raw : public std::string
    {
     public:
        inline raw(const std::string& src) : std::string{src} {}
        inline raw(std::string&& src) : std::string{std::move(src)} {}
    };

    enum undefined_t { undefined };
    enum null_t { null };

// ----------------------------------------------------------------------

    template <typename T1, typename T2> using is_same_ = std::is_same<T1, std::decay_t<T2>>;

    template <typename T> static inline constexpr bool is_const_char_ptr_v = is_same_<const char*, T>::value;
    template <typename T> static inline constexpr bool is_string_v = is_same_<std::string, T>::value;
    template <typename T> static inline constexpr bool is_raw_v = is_same_<raw, T>::value;

      // iterator SFINAE: https://stackoverflow.com/questions/12161109/stdenable-if-or-sfinae-for-iterator-or-pointer
    template <typename T> using if_iterator = decltype(*std::declval<T&>(), void(), ++std::declval<T&>(), void());

    template <typename T> using enable_if_not_const_char_ptr = std::enable_if_t<!is_const_char_ptr_v<T>>;

// ----------------------------------------------------------------------

    template <typename T> inline std::string value(const std::vector<T>& aVector);

    template <typename T> inline std::string value(T&& aValue)
    {
        if constexpr      (is_string_v<T>)                                       { return "\"" + aValue + "\""; }
        else if constexpr (is_raw_v<T>)                                          { return aValue; }
        else if constexpr (is_const_char_ptr_v<T>)                               { return std::string{"\""} + aValue + "\""; }
        else if constexpr (std::is_same<bool, std::decay_t<T>>::value)           { return aValue ? "true" : "false"; }
        else if constexpr (std::is_same<std::nullptr_t, std::decay_t<T>>::value) { return "null"; }
        else if constexpr (std::is_same<null_t, std::decay_t<T>>::value)         { return "null"; }
        else if constexpr (std::is_same<undefined_t, std::decay_t<T>>::value)    { return "undefined"; }
        else if constexpr (std::numeric_limits<std::decay_t<T>>::is_integer)     { return std::to_string(aValue); }
        else if constexpr (std::is_floating_point<std::decay_t<T>>::value)       { return acmacs::to_string(aValue); }
        else                                                                     { static_assert(std::is_same<int, std::decay_t<T>>::value, "use std::move?"); /* compilation fails trying to instantiate this type */ }
    }

// ----------------------------------------------------------------------

    namespace internal
    {
        template <typename Value, typename ... Args> inline std::string object_append(std::string target, std::string key, Value&& aValue, Args&& ... args)
        {
            if constexpr (sizeof...(args) == 0) {
                    const auto prefix = target.size() > 2 ? target.substr(0, target.size() - 1) + "," : std::string{"{"};
                    return prefix + "\"" + key + "\":" + value(std::forward<Value>(aValue)) + "}";
                }
            else {
                return object_append(object_append(target, key, std::forward<Value>(aValue)), std::forward<Args>(args) ...);
            }
        }

        inline std::string join(std::string left, std::string right)
        {
            if (left.size() < 3)
                return right;
            else if (right.size() < 3)
                return left;
            else
                return left.substr(0, left.size() - 1) + "," + right.substr(1);
        }

        template <typename Value, typename ... Args> inline std::string array_append(std::string target, Value&& aValue, Args&& ... args)
        {
            if constexpr (sizeof...(args) == 0) {
                    const auto prefix = target.size() > 2 ? target.substr(0, target.size() - 1) + "," : std::string{"["};
                    return prefix + value(std::forward<Value>(aValue)) + "]";
                }
            else {
                return array_append(array_append(target, std::forward<Value>(aValue)), std::forward<Args>(args) ...);
            }
        }

    } // namespace internal

// ----------------------------------------------------------------------

    template <typename ... Args> inline std::string object(Args&& ... args)
    {
        return internal::object_append(std::string{}, std::forward<Args>(args) ...);
    }

    template <typename Iterator, typename UnaryOperation, typename = if_iterator<Iterator>>
        inline std::string object(Iterator first, Iterator last, UnaryOperation unary_op)
    {
        std::string target = "{}";
        for (; first != last; ++first) {
            const auto [key, value] = unary_op(*first);
            target = internal::object_append(target, key, value);
        }
        return target;
    }

    template <typename Iterator, typename = if_iterator<Iterator>, typename = enable_if_not_const_char_ptr<Iterator>>
        inline std::string object(Iterator first, Iterator last)
    {
        std::string target = "{}";
        for (; first != last; ++first) {
            const auto [key, value] = *first;
            target = internal::object_append(target, key, value);
        }
        return target;
    }

    template <typename Value> inline std::string object(const std::map<std::string, Value>& aMap)
    {
        return object(std::begin(aMap), std::end(aMap));
    }

    template <typename ... Args> inline std::string object_append(std::string target, Args&& ... args)
    {
        return internal::join(target, object(std::forward<Args>(args) ...));
    }

    template <typename ... Args> inline std::string object_prepend(std::string target, Args&& ... args)
    {
        return internal::join(object(std::forward<Args>(args) ...), target);
    }

// ----------------------------------------------------------------------

    template <typename ... Args> inline std::string array(Args&& ... args)
    {
        return internal::array_append(std::string{}, std::forward<Args>(args) ...);
    }

    template <typename Iterator, typename = if_iterator<Iterator>>
        inline std::string array(Iterator first, Iterator last)
    {
        std::string target = "[]";
        for (; first != last; ++first) {
            target = internal::array_append(target, *first);
        }
        return target;
    }

    template <typename ... Args> inline std::string array_append(std::string target, Args&& ... args)
    {
        return internal::join(target, array(std::forward<Args>(args) ...));
    }

    template <typename ... Args> inline std::string array_prepend(std::string target, Args&& ... args)
    {
        return internal::join(array(std::forward<Args>(args) ...), target);
    }

    template <typename T> inline std::string value(const std::vector<T>& aVector)
    {
        return array(std::begin(aVector), std::end(aVector));
    }

// ----------------------------------------------------------------------

} // namespace to_json

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
