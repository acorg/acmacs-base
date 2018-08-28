#pragma once

#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>

#include "acmacs-base/to-string.hh"

// ----------------------------------------------------------------------

namespace rjson2
{
    class value;
    class array;
    class object;

    enum class json_pp_emacs_indent { no, yes };

    class merge_error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

      // --------------------------------------------------

    class null
    {
     // public:
     //    null() = default;
     //    null(const null&) = default;
     //    null& operator=(const null&) = default;

    }; // class null

      // --------------------------------------------------

    class object
    {
     public:
        object() = default;
          // object(std::initializer_list<std::pair<string, value>> key_values);
        // object(const object&) = default;
        // object& operator=(const object&) = default;
        // object(object&&) = default;
        // object& operator=(object&&) = default;

        template <typename S> value* find(S key) { const auto found = content_.find(key); return found == content_.end() ? nullptr : &found->second; }
        template <typename S> const value* find(S key) const { const auto found = content_.find(key); return found == content_.end() ? nullptr : &*found; }

        void insert(value&& aKey, value&& aValue);
        template <typename S> void insert(S aKey, const value& aValue);
        template <typename S> void remove(S key);
        void update(const object& to_merge);

        void remove_comments();

        friend std::string to_string(const object& val);

     private:
        std::map<std::string, value> content_;

    }; // class object

      // --------------------------------------------------

    class array
    {
     public:
        array() = default;

        value& insert(value&& aValue); // returns ref to inserted

        void remove_comments();

        friend std::string to_string(const array& val);

     private:
        std::vector<value> content_;

    }; // class array

      // --------------------------------------------------

    using number_base = std::variant<long, double, std::string>;

    class number : public number_base
    {
     public:
        using number_base::number_base;

        friend inline std::string to_string(const number& val)
        {
            auto visitor = [](auto&& arg) -> std::string {
                if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>)
                    return arg;
                else
                    return acmacs::to_string(arg);
            };
            return std::visit(visitor, val);
        }

    }; // class number<>

      // --------------------------------------------------

    using value_base = std::variant<null, object, array, std::string, number, bool>; // null must be the first alternative, it is the default value;

    class value : public value_base
    {
     public:
        using value_base::operator=;
        using value_base::value_base;
        value(const value&) = default;
        value(value&&) = default;
        value(std::string_view src) : value_base(std::string(src)) {}
        value& operator=(const value&) = default;
        value& operator=(value&&) = default;

        value& update(const value& to_merge);
        void remove_comments();

    }; // class value

      // --------------------------------------------------

    inline value& array::insert(value&& aValue)
    {
        content_.push_back(std::move(aValue));
        return content_.back();
    }

    inline void array::remove_comments()
    {
        std::for_each(content_.begin(), content_.end(), [](auto& val) { val.remove_comments(); });
    }

    inline void object::insert(value&& aKey, value&& aValue)
    {
        content_.emplace(std::get<std::string>(std::move(aKey)), std::move(aValue));
    }

    template <typename S> inline void object::insert(S aKey, const value& aValue)
    {
        content_.emplace(std::string(aKey), aValue);
    }

    inline void object::update(const object& to_merge)
    {
        for (const auto& [new_key, new_value] : to_merge.content_) {
            if (auto* old_value = find(new_key); old_value)
                old_value->update(new_value);
            else
                insert(new_key, new_value);
        }
    }

    inline void object::remove_comments()
    {
        auto is_comment_key = [](const std::string& key) -> bool { return !key.empty() && (key.front() == '?' || key.back() == '?'); };
        for (auto it = content_.begin(); it != content_.end(); /* no increment! */) {
            if (is_comment_key(it->first)) {
                it = content_.erase(it);
            }
            else {
                it->second.remove_comments();
                ++it;
            }
        }
    }

    // --------------------------------------------------

    class parse_error : public std::exception
    {
     public:
        parse_error(size_t line, size_t column, std::string&& message) : message_{std::to_string(line) + ":" + std::to_string(column) + ": " + std::move(message)} {}
        const char* what() const noexcept override { return message_.data(); }

     private:
        std::string message_;

    }; // class parse_error

    enum class remove_comments { no, yes };

    value parse_string(std::string data, remove_comments rc = remove_comments::yes);
    value parse_string(std::string_view data, remove_comments rc = remove_comments::yes);
    value parse_string(const char* data, remove_comments rc = remove_comments::yes);
    value parse_file(std::string filename, remove_comments rc = remove_comments::yes);

} // namespace rjson2

#ifndef __clang__
namespace std
{
      // gcc 7.3 wants those, if we derive from std::variant
    template<> struct variant_size<rjson2::value> : variant_size<rjson2::value_base> {};
    template<size_t _Np> struct variant_alternative<_Np, rjson2::value> : variant_alternative<_Np, rjson2::value_base> {};
}
#endif

namespace rjson2
{
    template <typename S> inline void object::remove(S key)
    {
        if (const auto found = content_.find(key); found != content_.end())
            content_.erase(found);
    }

    inline std::string to_string(const value& val)
    {
        auto visitor = [](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, null>)
                return "null";
            else if constexpr (std::is_same_v<T, std::string>)
                return "\"" + arg + '"';
            else if constexpr (std::is_same_v<T, bool>)
                return arg ? "true" : "false";
            else
                return to_string(arg);
        };
        return std::visit(visitor, val);
    }

    inline value& value::update(const value& to_merge)
    {
        auto visitor = [](auto& arg1, auto&& arg2) {
            using T1 = std::decay_t<decltype(arg1)>;
            using T2 = std::decay_t<decltype(arg2)>;
            if constexpr (std::is_same_v<T1, T2>) {
                if constexpr (std::is_same_v<T1, object>)
                    arg1.update(arg2);
                else
                    arg1 = arg2;
            }
            else
                throw merge_error(std::string{"cannot merge two rjson values of different types: %"}); //  + to_string(*this) + "% and %" + to_string(arg2));
        };

        std::visit(visitor, *this, to_merge);
        return *this;
    }

    inline void value::remove_comments()
    {
      std::visit([](auto&& arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, object> || std::is_same_v<T, array>)
              arg.remove_comments();
      },
      *this);
    }

} // namespace rjson2

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
