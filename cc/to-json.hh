#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>

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

// ----------------------------------------------------------------------

    namespace internal
    {
        template <typename T> inline std::string make_value(T value)
        {
            if constexpr      (std::is_same<std::string, T>::value)    { return "\"" + value + "\""; }
            else if constexpr (std::is_same<to_json::raw, T>::value)   { return value; }
            else if constexpr (std::is_same<const char*, T>::value)    { return std::string{"\""} + value + "\""; }
            else if constexpr (std::is_same<bool, T>::value)           { return value ? "true" : "false"; }
            else if constexpr (std::is_same<std::nullptr_t, T>::value) { return "null"; }
            else if constexpr (std::numeric_limits<T>::is_integer)     { return std::to_string(value); }
            else if constexpr (std::is_floating_point<T>::value)       { return double_to_string(value); }
            else                                                       { return std::to_string(value); }
        }

        // inline std::string make_value(std::string value) { return "\"" + value + "\""; }
        // inline std::string make_value(to_json::raw value) { return value; }
        // inline std::string make_value(const char* value) { return std::string{"\""} + value + "\""; }
        // inline std::string make_value(bool value) { return value ? "true" : "false"; }
        // inline std::string make_value(std::nullptr_t) { return "null"; }
        // template <typename Value> inline typename std::enable_if<std::numeric_limits<Value>::is_integer, std::string>::type make_value(Value value) { return std::to_string(value); }
        // template <typename Value> inline typename std::enable_if<std::is_floating_point<Value>::value, std::string>::type make_value(Value value) { return double_to_string(value); }

        template <typename Value, typename ... Args> inline std::string object_append(std::string target, std::string key, Value value, Args ... args)
        {
            if constexpr (sizeof...(args) == 0) {
                    return (target.size() > 2 ? target.substr(0, target.size() - 1) + "," : std::string{"{"}) + "\"" + key + "\":" + make_value(value) + "}";
                }
            else {
                return object_append(object_append(target, key, value), args ...);
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

        template <typename Value, typename ... Args> inline std::string array_append(std::string target, Value value, Args ... args)
        {
            if constexpr (sizeof...(args) == 0) {
                    return (target.size() > 2 ? target.substr(0, target.size() - 1) + "," : std::string{"["}) + internal::make_value(value) + "]";
                }
            else {
                return array_append(array_append(target, value), args ...);
            }
        }

        // template <typename Value> inline std::string object_append(std::string target, std::string key, Value value)
        // {
        //     std::string result = target.size() > 2 ? target.substr(0, target.size() - 1) + "," : std::string{"{"};
        //     result += "\"" + key + "\":" + make_value(value) + "}";
        //     return result;
        // }

        // template <typename Value, typename ... Args> inline std::string object_append(std::string target, std::string key, Value value, Args ... args)
        // {
        //     return object_append(object_append(target, key, value), args ...);
        // }

        // template <typename Value> inline std::string array_append(std::string target, Value value)
        // {
        //     std::string result = target.size() > 2 ? target.substr(0, target.size() - 1) + "," : std::string{"["};
        //     result += internal::make_value(value) + "]";
        //     return result;
        // }

        // template <typename Value, typename ... Args> inline std::string array_append(std::string target, Value value, Args ... args)
        // {
        //     return array_append(array_append(target, value), args ...);
        // }

    } // namespace internal

// ----------------------------------------------------------------------


    template <typename Value> inline std::string value(Value value)
    {
        return internal::make_value(value);
    }

    template <typename ... Args> inline std::string object(Args ... args)
    {
        return internal::object_append(std::string{}, args ...);
    }

    template <typename ... Args> inline std::string object_prepend(std::string target, Args ... args)
    {
        return internal::join(object(args ...), target);
    }

// ----------------------------------------------------------------------

    template <typename ... Args> inline std::string array(Args ... args)
    {
        return internal::array_append(std::string{}, args ...);
    }

    template <typename ... Args> inline std::string array_append(std::string target, Args ... args)
    {
        return internal::join(target, array(args ...));
    }

    template <typename ... Args> inline std::string array_prepend(std::string target, Args ... args)
    {
        return internal::join(array(args ...), target);
    }

// ----------------------------------------------------------------------

} // namespace to_json

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
