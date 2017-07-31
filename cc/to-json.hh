#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <typeinfo>

// ----------------------------------------------------------------------

#include "acmacs-base/float.hh"
#include "acmacs-base/rapidjson.hh"

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

    template <typename T> inline std::string value(T&& aValue)
    {
        if constexpr      (std::is_same<std::string, std::decay_t<T>>::value)    { return "\"" + aValue + "\""; }
        else if constexpr (std::is_same<to_json::raw, std::decay_t<T>>::value)   { return aValue; }
        else if constexpr (std::is_same<const char*, std::decay_t<T>>::value)    { return std::string{"\""} + aValue + "\""; }
        else if constexpr (std::is_same<bool, std::decay_t<T>>::value)           { return aValue ? "true" : "false"; }
        else if constexpr (std::is_same<std::nullptr_t, std::decay_t<T>>::value) { return "null"; }
        else if constexpr (std::is_same<null_t, std::decay_t<T>>::value)         { return "null"; }
        else if constexpr (std::is_same<undefined_t, std::decay_t<T>>::value)    { return "undefined"; }
        else if constexpr (std::numeric_limits<std::decay_t<T>>::is_integer)     { return std::to_string(aValue); }
        else if constexpr (std::is_floating_point<std::decay_t<T>>::value)       { return double_to_string(aValue); }
        else                                                                     { static_assert(std::is_same<int, std::decay_t<T>>::value, "use std::move?"); /* compilation fails trying to instantiate this type */ }
    }

// ----------------------------------------------------------------------

    namespace internal
    {
        template <typename Value, typename ... Args> inline std::string object_append(std::string target, std::string key, Value&& aValue, Args&& ... args)
        {
            if constexpr (sizeof...(args) == 0) {
                    return (target.size() > 2 ? target.substr(0, target.size() - 1) + "," : std::string{"{"}) + "\"" + key + "\":" + value(std::forward<Value>(aValue)) + "}";
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
                    return (target.size() > 2 ? target.substr(0, target.size() - 1) + "," : std::string{"["}) + value(std::forward<Value>(aValue)) + "]";
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

      // iterator SFINAE: https://stackoverflow.com/questions/12161109/stdenable-if-or-sfinae-for-iterator-or-pointer
    template <typename Iterator, typename UnaryOperation, typename = decltype(*std::declval<Iterator&>(), void(), ++std::declval<Iterator&>(), void())>
        inline std::string object(Iterator first, Iterator last, UnaryOperation unary_op)
    {
        std::string target;
        for (; first != last; ++first) {
            const auto [key, value] = unary_op(*first);
            internal::object_append(target, key, value);
        }
        return target;
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

    template <typename ... Args> inline std::string array_append(std::string target, Args&& ... args)
    {
        return internal::join(target, array(std::forward<Args>(args) ...));
    }

    template <typename ... Args> inline std::string array_prepend(std::string target, Args&& ... args)
    {
        return internal::join(array(std::forward<Args>(args) ...), target);
    }

// ----------------------------------------------------------------------

} // namespace to_json

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
