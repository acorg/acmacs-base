#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <typeinfo>

// ----------------------------------------------------------------------

#include "acmacs-base/sfinae.hh"
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

    enum Options {
        option_default,
        ignore_empty_values
    };

// ----------------------------------------------------------------------

    template <typename T> inline std::string value(const std::vector<T>& aVector);

    template <typename T> inline std::string value(T&& aValue)
    {
        using namespace acmacs::sfinae;
        using namespace std::string_literals;
        if constexpr (decay_equiv_v<T, std::string_view>)                    { return "\"" + std::string(aValue) + "\""; }
        else if constexpr (is_string_v<T>)                                   { return "\""s + aValue + '"'; }
        else if constexpr (decay_equiv_v<T, raw>)                            { return aValue; }
        else if constexpr (decay_equiv_v<T, bool>)                           { return aValue ? "true" : "false"; }
        else if constexpr (decay_equiv_v<T, std::nullptr_t>)                 { return "null"; }
        else if constexpr (decay_equiv_v<T, null_t>)                         { return "null"; }
        else if constexpr (decay_equiv_v<T, undefined_t>)                    { return "undefined"; }
        else if constexpr (std::numeric_limits<std::decay_t<T>>::is_integer) { return std::to_string(aValue); }
        else if constexpr (decay_equiv_v<T, double>)                         { return acmacs::to_string(aValue); }
        else if constexpr (decay_equiv_v<T, float>)                          { return std::to_string(aValue); }
          // else if constexpr (std::is_floating_point_v<std::decay_t<T>>)       { return acmacs::to_string(aValue); }
        else                                                                 { static_assert(std::is_same<int, std::decay_t<T>>::value, "use std::move?"); /* compilation fails trying to instantiate this type */ }
    }

// ----------------------------------------------------------------------

    namespace internal
    {
        template <typename Value, typename ... Args> inline std::string object_append(std::string target, Options options, std::string key, Value&& aValue, Args&& ... args)
        {
            if constexpr (sizeof...(args) == 0) {
                    const auto val = value(std::forward<Value>(aValue));
                    if (options == ignore_empty_values && (val == "\"\"" || val == "[]" || val == "{}" || val == "null"))
                        return target;
                    const auto prefix = target.size() > 2 ? target.substr(0, target.size() - 1) + "," : std::string{"{"};
                    return prefix + "\"" + key + "\":" + val + "}";
                }
            else {
                return object_append(object_append(target, options, key, std::forward<Value>(aValue)), options, std::forward<Args>(args) ...);
            }
        }

        template <typename Key, typename Value, typename ... Args> inline std::string object_append(std::string target, Options options, Key&& key, Value&& aValue, Args&& ... args)
        {
            return object_append(target, options, acmacs::to_string(key), std::forward<Value>(aValue), std::forward<Args>(args) ...);
        }

        // template <typename Value, typename ... Args> inline std::string object_append(std::string target, std::string key, Value&& aValue, Args&& ... args)
        // {
        //     return object_append(target, option_default, key, std::forward<Value>(aValue));
        // }

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

    template <typename Iterator, typename UnaryOperation, typename = acmacs::sfinae::iterator_t<Iterator>>
        inline std::string object(Iterator first, Iterator last, UnaryOperation unary_op, Options options = option_default)
    {
        std::string target = "{}";
        for (; first != last; ++first) {
            const auto [key, value] = unary_op(*first);
            target = internal::object_append(target, options, key, value);
        }
        return target;
    }

    template <typename Iterator, typename = acmacs::sfinae::iterator_t<Iterator>, typename = std::enable_if_t<!acmacs::sfinae::is_const_char_ptr_v<Iterator>>>
        inline std::string object(Iterator first, Iterator last, Options options = option_default)
    {
        std::string target = "{}";
        for (; first != last; ++first) {
            const auto [key, value] = *first;
            target = internal::object_append(target, options, key, value);
        }
        return target;
    }

    namespace internal
    {
        template <typename Arg> inline std::string object1(Arg&& arg)
        {
            return to_json::object(std::begin(arg), std::end(arg));
        }
    }

    template <typename... Args> inline std::string object(Args&&... args)
    {
        if constexpr (sizeof...(args) == 1) {
            return internal::object1(std::forward<Args>(args)...); // for to_json::object(const std::map<Key, Value>& aMap)
        }
        else if constexpr (sizeof...(args) > 0)
            return internal::object_append(std::string{}, option_default, std::forward<Args>(args)...);
        else
            return "{}";
    }

    template <typename ... Args> inline std::string object(Options options, Args&& ... args)
    {
        return internal::object_append(std::string{}, options, std::forward<Args>(args) ...);
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

    template <typename Iterator, typename = acmacs::sfinae::iterator_t<Iterator>>
        inline std::string array(Iterator first, Iterator last)
    {
        std::string target = "[]";
        for (; first != last; ++first) {
            target = internal::array_append(target, *first);
        }
        return target;
    }

    template <typename Iterator, typename Transformer, typename = acmacs::sfinae::iterator_t<Iterator>>
        inline std::string array(Iterator first, Iterator last, Transformer transformer)
    {
        std::string target = "[]";
        for (; first != last; ++first) {
            target = internal::array_append(target, transformer(*first));
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
