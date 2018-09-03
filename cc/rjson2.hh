#pragma once

#ifdef RJSON1
#error cannot use rjson2, rjson1 already included
#endif
#ifndef RJSON2
#define RJSON2
#endif

#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <limits>

#include "acmacs-base/to-string.hh"

// ----------------------------------------------------------------------

namespace rjson2
{
    class value;
    class array;
    class object;

    enum class json_pp_emacs_indent { no, yes };

    class value_type_mismatch : public std::runtime_error { public: value_type_mismatch(std::string requested_type, std::string actual_type) : std::runtime_error{"value type mismatch, requested: " + requested_type + ", stored: " + actual_type} {} };
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
        object(std::initializer_list<std::pair<std::string, value>> key_values);
        // object(const object&) = default;
        // object& operator=(const object&) = default;
        // object(object&&) = default;
        // object& operator=(object&&) = default;

        bool empty() const noexcept { return content_.empty(); }

        template <typename S> const value& get(S key) const noexcept;
        template <typename S> value& operator[](S key) noexcept;

        void insert(value&& aKey, value&& aValue);
        template <typename S> void insert(S aKey, const value& aValue);
        template <typename S> void remove(S key);
        void update(const object& to_merge);

        void remove_comments();

        template <typename Func> inline bool all_of(Func func) const { return std::all_of(content_.begin(), content_.end(), func); }

     private:
        std::map<std::string, value> content_;

        friend std::string to_string(const object& val, bool space_after_comma);
        friend std::string pretty(const object& val, size_t indent, json_pp_emacs_indent emacs_indent, size_t prefix);

    }; // class object

      // --------------------------------------------------

    class array
    {
     public:
        array() = default;
        array(std::initializer_list<value> init) : content_(init) {}
        template <typename Iterator> array(Iterator first, Iterator last) : content_(first, last) {}

        bool empty() const noexcept { return content_.empty(); }

        const value& get(size_t index) const noexcept; // if index out of range, returns ConstNull
        value& operator[](size_t index) noexcept;      // if index out of range, returns ConstNull

        value& insert(value&& aValue); // returns ref to inserted

        void remove_comments();

        template <typename Func> inline bool all_of(Func func) const { return std::all_of(content_.begin(), content_.end(), func); }

     private:
        std::vector<value> content_;

        friend std::string to_string(const array& val, bool space_after_comma);
        friend std::string pretty(const array& val, size_t indent, json_pp_emacs_indent emacs_indent, size_t prefix);

    }; // class array

      // --------------------------------------------------

    using number = std::variant<long, double, std::string>;

    inline std::string to_string(const number& val)
    {
        auto visitor = [](auto&& arg) -> std::string {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>)
                return arg;
            else
                return acmacs::to_string(arg);
        };
        return std::visit(visitor, val);
    }

    inline double to_double(const number& val)
    {
        auto visitor = [](auto&& arg) -> double {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, long>)
                return static_cast<double>(arg);
            else if constexpr (std::is_same_v<T, double>)
                return arg;
            else if constexpr (std::is_same_v<T, std::string>)
                return std::stod(arg);
            else
                return std::numeric_limits<double>::quiet_NaN();
        };
        return std::visit(visitor, val);
    }

    inline size_t to_size_t(const number& val)
    {
        auto visitor = [](auto&& arg) -> size_t {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, long>)
                return static_cast<size_t>(arg);
            else if constexpr (std::is_same_v<T, double>)
                return static_cast<size_t>(std::lround(arg));
            else if constexpr (std::is_same_v<T, std::string>)
                return std::stoul(arg);
            else
                return std::numeric_limits<size_t>::max();
        };
        return std::visit(visitor, val);
    }

    inline long to_long(const number& val)
    {
        auto visitor = [](auto&& arg) -> long {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, long>)
                return arg;
            else if constexpr (std::is_same_v<T, double>)
                return std::lround(arg);
            else if constexpr (std::is_same_v<T, std::string>)
                return std::stol(arg);
            else
                return std::numeric_limits<long>::max();
        };
        return std::visit(visitor, val);
    }

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
        value(const char* src) : value_base(std::string(src)) {}
        value(char* src) : value_base(std::string(src)) {}
        template <typename Uint, typename std::enable_if<std::is_integral<Uint>::value>::type* = nullptr> value(Uint src) : value_base(number(static_cast<long>(src))) {}
        template <typename Dbl, typename std::enable_if<std::is_floating_point<Dbl>::value>::type* = nullptr> value(Dbl src) : value_base(number(static_cast<double>(src))) {}
        value(int src) : value_base(number(static_cast<long>(src))) {} // gcc 7.2 wants it to disambiguate
        value(double src) : value_base(number(src)) {} // gcc 7.2 wants it to disambiguate
        value(bool src) : value_base(src) {}
        value& operator=(const value&) = default;
        value& operator=(value&&) = default;

        bool empty() const noexcept;
        bool is_null() const noexcept;
        const value& get(std::string field_name) const noexcept; // if this is not object or field not present, returns ConstNull
        value& operator[](std::string field_name) noexcept;      // if this is not object, returns ConstNull; if field not present, inserts field with null value and returns it
        const value& operator[](std::string field_name) const  noexcept{ return get(field_name); }
        const value& get(size_t index) const noexcept; // if this is not array or index out of range, returns ConstNull
        value& operator[](size_t index) noexcept;      // if this is not array or index out of range, returns ConstNull
        const value& operator[](size_t index) const noexcept { return get(index); }

        operator const std::string&() const;
        operator double() const;
        operator size_t() const;
        operator long() const;
        operator bool() const;

        template <typename Func> const value& find_if(Func func) const; // returns ConstNull if not found, Func: bool (const value&)
        template <typename Func> value& find_if(Func func) const; // returns ConstNull if not found, Func: bool (value&)

        value& update(const value& to_merge);
        void remove_comments();

     private:
        std::string actual_type() const;

    }; // class value

      // --------------------------------------------------

    class const_null : public value
    {
     public:
        const_null() = default;
        const_null(const const_null&) = delete;
        const_null(const_null&&) = delete;
        const_null& operator=(const const_null&) = delete;
        const_null& operator=(const_null&&) = delete;
    };

    extern const_null ConstNull;

    // --------------------------------------------------

    inline const value& array::get(size_t index) const noexcept // if index out of range, returns ConstNull
    {
        if (index < content_.size())
            return content_[index];
        else
            return ConstNull;
    }

    inline value& array::operator[](size_t index) noexcept      // if index out of range, returns ConstNull
    {
        if (index < content_.size())
            return content_[index];
        else
            return ConstNull;
    }

    inline value& array::insert(value&& aValue)
    {
        content_.push_back(std::move(aValue));
        return content_.back();
    }

    inline void array::remove_comments()
    {
        std::for_each(content_.begin(), content_.end(), [](auto& val) { val.remove_comments(); });
    }

    inline object::object(std::initializer_list<std::pair<std::string, value>> key_values) : content_(std::begin(key_values), std::end(key_values)) {}

    template <typename S> inline const value& object::get(S key) const noexcept
    {
        if (const auto found = content_.find(key); found != content_.end())
            return found->second;
        else
            return ConstNull;
    }

    template <typename S> inline value& object::operator[](S key) noexcept
    {
        return content_.emplace(std::string(key), value{}).first->second;
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
        for (const auto& [new_key, new_value] : to_merge.content_)
            operator[](new_key).update(new_value);
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

    std::string to_string(const object& val, bool space_after_comma = false);
    std::string to_string(const array& val, bool space_after_comma = false);

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

    std::string pretty(const value& val, size_t indent = 2, json_pp_emacs_indent emacs_indent = json_pp_emacs_indent::yes, size_t prefix = 0);
    std::string pretty(const object& val, size_t indent, json_pp_emacs_indent emacs_indent, size_t prefix);
    std::string pretty(const array& val, size_t indent, json_pp_emacs_indent emacs_indent, size_t prefix);

    inline std::string pretty(const value& val, size_t indent, json_pp_emacs_indent emacs_indent, size_t prefix)
    {
        auto visitor = [&val,indent,emacs_indent,prefix](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, object> || std::is_same_v<T, array>)
                return pretty(arg, indent, emacs_indent, prefix);
            else
                return to_string(val);
        };
        return std::visit(visitor, val);
    }

    inline std::string value::actual_type() const
    {
        return std::visit([](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, null>)
                return "null";
            else if (std::is_same_v<T, object>)
                return "object";
            else if (std::is_same_v<T, array>)
                return "array";
            else if (std::is_same_v<T, std::string>)
                return "std::string";
            else if (std::is_same_v<T, number>)
                return "number";
            else if (std::is_same_v<T, bool>)
                return "bool";
            else
                return "*unknown*";
        }, *this);
    }

    inline bool value::empty() const noexcept
    {
        return std::visit([](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, object> || std::is_same_v<T, array> || std::is_same_v<T, std::string>)
                return arg.empty();
            else if (std::is_same_v<T, null>)
                return true;
            else
                return false;
        }, *this);
    }

    inline bool value::is_null() const noexcept
    {
        return std::visit([](auto&& arg) { if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, null>) return true; else return false; }, *this);
    }

    inline const value& value::get(std::string field_name) const noexcept // if this is not object or field not present, returns ConstNull
    {
        return std::visit([&field_name](auto&& arg) -> const value& {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, object>)
                return arg.get(field_name);
            else
                return ConstNull;
        }, *this);
    }

    inline value& value::operator[](std::string field_name) noexcept      // if this is not object, returns ConstNull; if field not present, inserts field with null value and returns it
    {
        return std::visit([&field_name](auto&& arg) -> value& {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, object>)
                return arg[field_name];
            else
                return ConstNull;
        }, *this);
    }

    inline const value& value::get(size_t index) const noexcept // if this is not object or field not present, returns ConstNull
    {
        return std::visit([index](auto&& arg) -> const value& {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, array>)
                return arg.get(index);
            else
                return ConstNull;
        }, *this);
    }

    inline value& value::operator[](size_t index) noexcept
    {
        return std::visit([index](auto&& arg) -> value& {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, array>)
                return arg[index];
            else
                return ConstNull;
        }, *this);
    }

    inline value::operator const std::string&() const
    {
        return std::visit([this](auto&& arg) -> const std::string& {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>)
                return arg;
            else
                throw value_type_mismatch("std::string", actual_type());
        }, *this);
    }

    inline value::operator double() const
    {
        return std::visit([this](auto&& arg) -> double {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number>)
                return to_double(arg);
            else
                throw value_type_mismatch("number", actual_type());
        }, *this);
    }

    inline value::operator size_t() const
    {
        return std::visit([this](auto&& arg) -> size_t {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number>)
                return to_size_t(arg);
            else
                throw value_type_mismatch("number", actual_type());
        }, *this);
    }

    inline value::operator long() const
    {
        return std::visit([this](auto&& arg) -> long {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number>)
                return to_long(arg);
            else
                throw value_type_mismatch("number", actual_type());
        }, *this);
    }

    inline value::operator bool() const
    {
        return std::visit([this](auto&& arg) -> bool {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, bool>)
                return arg;
            else
                throw value_type_mismatch("bool", actual_type());
        }, *this);
    }


    template <typename Func> inline const value& value::find_if(Func func) const // returns ConstNull if not found, Func: bool (const value&)
    {
        return std::visit([&func,this](auto&& arg) -> const value& {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, object> || std::is_same_v<T, array>)
                return arg.find_if(func);
            else if (func(*this))
                return *this;
            else
                return ConstNull;
        }, *this);
    }

    template <typename Func> inline value& value::find_if(Func func) const // returns ConstNull if not found, Func: bool (value&)
    {
        return std::visit([&func,this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, object> || std::is_same_v<T, array>)
                return arg.find_if(func);
            else if (func(*this))
                return *this;
            else
                return ConstNull;
        }, *this);
    }

    inline value& value::update(const value& to_merge)
    {
        auto visitor = [this](auto& arg1, auto&& arg2) {
            using T1 = std::decay_t<decltype(arg1)>;
            using T2 = std::decay_t<decltype(arg2)>;
            if constexpr (std::is_same_v<T1, T2>) {
                if constexpr (std::is_same_v<T1, object>)
                    arg1.update(arg2);
                else
                    arg1 = arg2;
            }
            else if constexpr (std::is_same_v<T1, null>)
                *this = arg2;
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
