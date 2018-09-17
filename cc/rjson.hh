#pragma once

#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <limits>

#include "acmacs-base/sfinae.hh"
#include "acmacs-base/debug.hh"
#include "acmacs-base/to-string.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-base/rjson-forward.hh"
#include "acmacs-base/rjson-v1.hh"

// ----------------------------------------------------------------------

namespace rjson
{
    inline namespace v2
    {
        class value;
        class array;
        class object;

        enum class json_pp_emacs_indent { no, yes };

        class value_type_mismatch : public std::runtime_error
        {
          public:
            value_type_mismatch(std::string requested_type, std::string actual_type, std::string source_ref)
                : std::runtime_error{"value type mismatch, requested: " + requested_type + ", stored: " + actual_type + source_ref}
            {
            }
        };
        class merge_error : public std::runtime_error
        {
          public:
            using std::runtime_error::runtime_error;
        };
        class const_null_modification_attempt : public std::runtime_error
        {
          public:
            const_null_modification_attempt() : std::runtime_error{"ConstNull modification attempt"} {}
        };

        // --------------------------------------------------

        class null
        {
        };

        class const_null
        {
        };

        // --------------------------------------------------

        class object
        {
          public:
            using content_t = std::map<std::string, value>;
            using value_type = typename content_t::value_type;

            object() = default;
            object(std::initializer_list<value_type> key_values);
            // object(const object&) = default;
            // object& operator=(const object&) = default;
            // object(object&&) = default;
            // object& operator=(object&&) = default;

            bool empty() const noexcept { return content_.empty(); }
            size_t size() const noexcept { return content_.size(); }

            template <typename S> const value& get(S key) const noexcept;
            template <typename S> value& operator[](S key) noexcept;
            size_t max_index() const;

            void insert(value&& aKey, value&& aValue);
            template <typename S> void insert(S aKey, const value& aValue);
            template <typename S> void remove(S key);
            void update(const object& to_merge);

            void remove_comments();

            template <typename Func> inline bool all_of(Func func) const { return std::all_of(content_.begin(), content_.end(), func); }

            template <typename T> void copy_to(T&& target) const;
            template <typename T, typename F> void transform_to(T&& target, F&& transformer) const;
            template <typename F> void for_each(F&& func) const;
            template <typename F> void for_each(F&& func);

          private:
            content_t content_;

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
            size_t size() const noexcept { return content_.size(); }

            const value& get(size_t index) const noexcept; // if index out of range, returns ConstNull
            value& operator[](size_t index) noexcept;      // if index out of range, returns ConstNull
            size_t max_index() const { return content_.size() - 1; }

            value& append(value&& aValue); // returns ref to inserted

            void remove_comments();

            template <typename Func> inline bool all_of(Func func) const { return std::all_of(content_.begin(), content_.end(), func); }

            template <typename T> void copy_to(T&& target) const;
            template <typename T, typename F> void transform_to(T&& target, F&& transformer) const;
            template <typename F> void for_each(F&& func) const;
            template <typename F> void for_each(F&& func);
            template <typename Func> const value& find_if(Func func) const; // returns ConstNull if not found, Func: bool (const value&)
            template <typename Func> value& find_if(Func func);             // returns ConstNull if not found, Func: bool (value&)

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
                if constexpr (acmacs::sfinae::decay_equiv_v<decltype(arg), std::string>)
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

            template <typename T> T to_integer(const number& val)
            {
                auto visitor = [](auto&& arg) -> T {
                    using Arg = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<Arg, long>)
                        return static_cast<T>(arg);
                    else if constexpr (std::is_same_v<Arg, double>)
                        return static_cast<T>(std::lround(arg));
                    else if constexpr (std::is_same_v<Arg, std::string>)
                        return static_cast<T>(std::stoul(arg));
                    else
                        return std::numeric_limits<T>::max();
                };
                return std::visit(visitor, val);
            }

        // --------------------------------------------------

        using value_base = std::variant<null, const_null, object, array, std::string, number, bool>; // null must be the first alternative, it is the default value;

        class value : public value_base
        {
         private:
            template <typename T, typename = std::enable_if_t<std::is_same_v<std::decay_t<T>, char>>>
                value& assign(acmacs::sfinae::dispatching_priority<5>, T&& src)
                {
                    check_const_null();
                    value_base::operator=(std::string(1, src));
                    return *this;
                }

            template <typename T, typename = std::enable_if_t<(acmacs::sfinae::decay_equiv_v<T, std::string_view> || acmacs::sfinae::is_const_char_ptr_v<T>)>>
                value& assign(acmacs::sfinae::dispatching_priority<4>, T&& src)
                {
                    check_const_null();
                    value_base::operator=(std::string(std::forward<T>(src)));
                    return *this;
                }

            template <typename Float, typename = std::enable_if_t<std::is_floating_point_v<std::decay_t<Float>>>>
                value& assign(acmacs::sfinae::dispatching_priority<2>, Float&& src)
                {
                    check_const_null();
                    value_base::operator=(number(static_cast<double>(src)));
                    return *this;
                }

            template <typename Int, typename = std::enable_if_t<(std::is_integral_v<std::decay_t<Int>> && !std::is_same_v<std::decay_t<Int>, bool> && !std::is_same_v<std::decay_t<Int>, char>)>>
                value& assign(acmacs::sfinae::dispatching_priority<1>, Int&& src)
                {
                    check_const_null();
                    value_base::operator=(number(static_cast<long>(src)));
                    return *this;
                }

            template <typename T, typename = std::enable_if_t<(acmacs::sfinae::decay_equiv_v<T, bool> || std::is_base_of_v<std::string, std::decay_t<T>>
                                                               || acmacs::sfinae::decay_equiv_v<T, value> || acmacs::sfinae::decay_equiv_v<T, object> || acmacs::sfinae::decay_equiv_v<T, array>
                                                               || acmacs::sfinae::decay_equiv_v<T, number> || acmacs::sfinae::decay_equiv_v<T, null> || acmacs::sfinae::decay_equiv_v<T, const_null>)>>
                value& assign(acmacs::sfinae::dispatching_priority<0>, T&& src)
                {
                    check_const_null();
                    value_base::operator=(std::forward<T>(src));
                    return *this;
                }

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
            value(double src) : value_base(number(src)) {}                 // gcc 7.2 wants it to disambiguate
            value(bool src) : value_base(src) {}

            value& operator=(const value&) = default;
            value& operator=(value&&) = default;
            template <typename T> value& operator=(T&& src)
                {
                    return assign(acmacs::sfinae::dispatching_priority_top{}, std::forward<T>(src));
                }

            bool is_null() const noexcept;
            bool is_object() const noexcept;
            bool is_array() const noexcept;
            bool is_string() const noexcept;
            bool is_number() const noexcept;
            bool is_bool() const noexcept;

            bool empty() const noexcept;
            size_t size() const noexcept; // returns 0 if neither array nor object nor string
            template <typename S, typename = std::enable_if_t<acmacs::sfinae::is_string_v<S>>> value& operator[](S field_name) noexcept;      // if this is not object, returns ConstNull; if field not present, inserts field with null value and returns it
            const value& get(size_t index) const noexcept; // if this is not array or index out of range, returns ConstNull
            template <typename S, typename... Args, typename = std::enable_if_t<acmacs::sfinae::is_string_v<S>>> const value& get(S field_name, Args&&... args) const noexcept;
            template <typename S, typename... Args, typename = std::enable_if_t<acmacs::sfinae::is_string_v<S>>> value& set(S field_name, Args&&... args) noexcept; // creates intermediate objects, if necessary
            value& operator[](size_t index) noexcept; // if this is not array or index out of range, returns ConstNull
            const value& operator[](size_t index) const noexcept { return get(index); }
            template <typename S, typename = std::enable_if_t<acmacs::sfinae::is_string_v<S>>> const value& operator[](S field_name) const noexcept { return get(field_name); }
            value& append(value&& aValue); // for array only, returns ref to inserted
            value& append(double aValue) { return append(number(aValue)); }
            size_t max_index() const; // returns (size-1) for array, assumes object keys are size_t and returns max of them

            operator std::string() const;
            operator std::string_view() const;
            operator double() const;
            operator size_t() const { return to_integer<size_t>(); }
            operator long() const { return to_integer<long>(); }
            operator int() const { return to_integer<int>(); }
            operator unsigned() const { return to_integer<unsigned>(); }
            operator short() const { return to_integer<short>(); }
            operator unsigned short() const { return to_integer<unsigned short>(); }
            operator bool() const;
            template <typename R> R get_or_default(R&& dflt) const;
            std::string get_or_default(const char* dflt) const { return get_or_default(std::string(dflt)); }

            value& update(const value& to_merge);
            void remove_comments();

            std::string actual_type() const;

          private:
            template <typename S> const value& get1(S field_name) const noexcept; // if this is not object or field not present, returns ConstNull
            template <typename T> T to_integer() const;
            bool is_const_null() const noexcept;
            void check_const_null() const
            {
                if (is_const_null())
                    throw const_null_modification_attempt{};
            }

        }; // class value

        // --------------------------------------------------

        extern value ConstNull, EmptyArray, EmptyObject;

        // --------------------------------------------------

        inline const value& array::get(size_t index) const noexcept // if index out of range, returns ConstNull
        {
            if (index < content_.size())
                return content_[index];
            else
                return ConstNull;
        }

        inline value& array::operator[](size_t index) noexcept // if index out of range, returns ConstNull
        {
            if (index < content_.size())
                return content_[index];
            else
                return ConstNull;
        }

        inline value& array::append(value&& aValue)
        {
            content_.push_back(std::move(aValue));
            return content_.back();
        }

        inline void array::remove_comments()
        {
            std::for_each(content_.begin(), content_.end(), [](auto& val) { val.remove_comments(); });
        }

        template <typename T> inline void array::copy_to(T&& target) const
        {
            if constexpr (acmacs::sfinae::container_has_iterator<T>) {
                if constexpr (acmacs::sfinae::container_has_resize<T>)
                    target.resize(size());
                using dest_t = decltype(*target.begin());
                if constexpr (std::is_convertible_v<const value&, dest_t>)
                    std::transform(content_.begin(), content_.end(), target.begin(), [](const value& val) -> dest_t { return val; });
                else
                    std::transform(content_.begin(), content_.end(), target.begin(), [](const value& val) -> std::decay_t<dest_t> { return val; });
            }
            else {
                std::transform(content_.begin(), content_.end(), std::forward<T>(target), [](const value& val) -> std::remove_reference_t<decltype(*target)> { return val; });
            }
        }

        template <typename F> inline void array::for_each(F&& func) const
        {
            for (auto [no, val] : acmacs::enumerate(content_)) {
                if constexpr (std::is_invocable_v<F, const value&>)
                    func(val);
                else if constexpr (std::is_invocable_v<F, const value&, size_t>)
                    func(val, no);
                else
                    static_assert(std::is_invocable_v<F, const value&>, "array::for_each: unsupported func signature");
            }
        }

        template <typename F> inline void array::for_each(F&& func)
        {
            for (auto [no, val] : acmacs::enumerate(content_)) {
                if constexpr (std::is_invocable_v<F, value&>)
                    func(val);
                else if constexpr (std::is_invocable_v<F, value&, size_t>)
                    func(val, no);
                else
                    static_assert(std::is_invocable_v<F, value&>, "array::for_each: unsupported func signature");
            }
        }

        template <typename T, typename F> inline void array::transform_to(T&& target, F&& transformer) const
        {
            if constexpr (std::is_invocable_v<F, const value&>) {
                std::transform(content_.begin(), content_.end(), std::forward<T>(target), std::forward<F>(transformer));
            }
            else if constexpr (std::is_invocable_v<F, const value&, size_t>) {
                for (auto [no, src] : acmacs::enumerate(content_))
                    *target++ = transformer(src, no);
            }
            else {
                static_assert(std::is_invocable_v<F, const value&>, "rjson::array::transform_to: unsupported transformer signature");
            }
        }

        template <typename Func> inline const value& array::find_if(Func func) const { return *std::find_if(content_.begin(), content_.end(), func); }

        template <typename Func> inline value& array::find_if(Func func) { return *std::find_if(content_.begin(), content_.end(), func); }

        // --------------------------------------------------

        inline object::object(std::initializer_list<value_type> key_values) : content_(std::begin(key_values), std::end(key_values)) {}

        template <typename S> inline const value& object::get(S key) const noexcept
        {
            if (const auto found = content_.find(acmacs::to_string(key)); found != content_.end())
                return found->second;
            else
                return ConstNull;
        }

        template <typename S> inline value& object::operator[](S key) noexcept { return content_.emplace(acmacs::to_string(key), value{}).first->second; }

        inline size_t object::max_index() const // assumes keys are size_t
        {
            size_t result = 0;
            for ([[maybe_unused]] const auto& [key, _] : content_)
                result = std::max(std::stoul(key), result);
            return result;
        }

        inline void object::insert(value&& aKey, value&& aValue) { content_.emplace(std::get<std::string>(std::move(aKey)), std::move(aValue)); }

        template <typename S> inline void object::insert(S aKey, const value& aValue) { content_.emplace(acmacs::to_string(aKey), aValue); }

        template <typename S> inline void object::remove(S key)
        {
            if (const auto found = content_.find(acmacs::to_string(key)); found != content_.end())
                content_.erase(found);
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

        template <typename F> inline void object::for_each(F&& func) const { std::for_each(content_.begin(), content_.end(), std::forward<F>(func)); }

        template <typename F> inline void object::for_each(F&& func) { std::for_each(content_.begin(), content_.end(), std::forward<F>(func)); }

        template <typename T, typename F> inline void object::transform_to(T&& target, F&& transformer) const
        {
            if constexpr (acmacs::sfinae::container_has_iterator<T>) {
                if constexpr (acmacs::sfinae::container_has_resize<T>)
                    target.resize(content_.size());
                std::transform(content_.begin(), content_.end(), target.begin(), std::forward<F>(transformer));
            }
            else {
                std::transform(content_.begin(), content_.end(), std::forward<T>(target), std::forward<F>(transformer));
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

    } // namespace v2
} // namespace rjson

#ifndef __clang__
namespace std
{
      // gcc 7.3 wants those, if we derive from std::variant
    template<> struct variant_size<rjson::v2::value> : variant_size<rjson::v2::value_base> {};
    template<size_t _Np> struct variant_alternative<_Np, rjson::v2::value> : variant_alternative<_Np, rjson::v2::value_base> {};
}
#endif

namespace rjson
{
    inline namespace v2
    {
        std::string to_string(const object& val, bool space_after_comma = false);
        std::string to_string(const array& val, bool space_after_comma = false);

        inline std::string to_string(const value& val)
        {
            auto visitor = [](auto&& arg) -> std::string {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, null>)
                    return "null";
                else if constexpr (std::is_same_v<T, const_null>)
                    return "*ConstNull";
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
            auto visitor = [&val, indent, emacs_indent, prefix](auto&& arg) -> std::string {
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
            return std::visit(
                [](auto&& arg) -> std::string {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, null>)
                        return "null";
                    else if constexpr (std::is_same_v<T, const_null>)
                        return "ConstNull";
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
                },
                *this);
        }

        inline bool value::empty() const noexcept
        {
            return std::visit(
                [](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, object> || std::is_same_v<T, array> || std::is_same_v<T, std::string>)
                        return arg.empty();
                    else if (std::is_same_v<T, null> || std::is_same_v<T, const_null>)
                        return true;
                    else
                        return false;
                },
                *this);
        }

        inline size_t value::size() const noexcept
        {
            return std::visit(
                [](auto&& arg) -> size_t {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, object> || std::is_same_v<T, array> || std::is_same_v<T, std::string>)
                        return arg.size();
                    else
                        return 0;
                },
                *this);
        }

        inline bool value::is_null() const noexcept
        {
            return std::visit(
                [](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, null> || std::is_same_v<T, const_null>)
                        return true;
                    else
                        return false;
                },
                *this);
        }

        inline bool value::is_const_null() const noexcept
        {
            return std::visit(
                [](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, const_null>)
                        return true;
                    else
                        return false;
                },
                *this);
        }

        inline bool value::is_object() const noexcept
        {
            return std::visit([](auto&& arg) { if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, object>) return true; else return false; }, *this);
        }

        inline bool value::is_array() const noexcept
        {
            return std::visit([](auto&& arg) { if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, array>) return true; else return false; }, *this);
        }

        inline bool value::is_string() const noexcept
        {
            return std::visit([](auto&& arg) { if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>) return true; else return false; }, *this);
        }

        inline bool value::is_number() const noexcept
        {
            return std::visit([](auto&& arg) { if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number>) return true; else return false; }, *this);
        }

        inline bool value::is_bool() const noexcept
        {
            return std::visit([](auto&& arg) { if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, bool>) return true; else return false; }, *this);
        }

        template <typename S> inline const value& value::get1(S field_name) const noexcept // if this is not object or field not present, returns ConstNull
        {
            return std::visit(
                [&field_name](auto&& arg) -> const value& {
                    if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, object>)
                        return arg.get(field_name);
                    else
                        return ConstNull;
                },
                *this);
        }

        template <typename S, typename... Args, typename> const value& value::get(S field_name, Args&&... args) const noexcept
        {
            if (const auto& r1 = get1(field_name); !r1.is_null()) {
                if constexpr (sizeof...(args) > 0)
                    return r1.get(args...);
                else
                    return r1;
            }
            else
                return ConstNull;
        }

        // creates intermediate objects, if necessary.
        // if intermediate value exists and it is neither object nor null, returns ConstNull
        template <typename S, typename... Args, typename> value& value::set(S field_name, Args&&... args) noexcept
        {
            auto& r1 = operator[](field_name);
            if (r1.is_null())
                r1 = object{};
            else if (!r1.is_object())
                return ConstNull;
            if constexpr (sizeof...(args) > 0)
                return r1.set(args...);
            else
                return r1;
        }

        template <typename S, typename>
        inline value& value::operator[](S field_name) noexcept // if this is not object, returns ConstNull; if field not present, inserts field with null value and returns it
        {
            return std::visit(
                [&field_name](auto&& arg) -> value& {
                    if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, object>)
                        return arg[field_name];
                    else
                        return ConstNull;
                },
                *this);
        }

        inline const value& value::get(size_t index) const noexcept // if this is not object or field not present, returns ConstNull
        {
            return std::visit(
                [index](auto&& arg) -> const value& {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, array>)
                        return arg.get(index);
                    else if constexpr (std::is_same_v<T, object>)
                        return arg.get(std::to_string(index));
                    else
                        return ConstNull;
                },
                *this);
        }

        inline value& value::operator[](size_t index) noexcept
        {
            return std::visit(
                [index](auto&& arg) -> value& {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, array>)
                        return arg[index];
                    else if constexpr (std::is_same_v<T, object>)
                        return arg[std::to_string(index)];
                    else
                        return ConstNull;
                },
                *this);
        }

        inline value& value::append(value&& aValue)
        {
            return std::visit(
                [&aValue, this](auto&& arg) -> value& {
                    if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, array>)
                        return arg.append(std::move(aValue));
                    else
                        throw value_type_mismatch("array", actual_type(), DEBUG_LINE_FUNC);
                },
                *this);
        }

        inline value::operator std::string() const
        {
            return std::visit(
                [this](auto&& arg) -> std::string {
                    if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>)
                        return arg;
                    else
                        throw value_type_mismatch("std::string", actual_type(), DEBUG_LINE_FUNC);
                },
                *this);
        }

        inline value::operator std::string_view() const
        {
            return std::visit(
                [this](auto&& arg) -> std::string_view {
                    if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::string>)
                        return std::string_view(arg);
                    else
                        throw value_type_mismatch("std::string", actual_type(), DEBUG_LINE_FUNC);
                },
                *this);
        }

        inline value::operator double() const
        {
            return std::visit(
                [this](auto&& arg) -> double {
                    if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number>)
                        return to_double(arg);
                    else
                        throw value_type_mismatch("number", actual_type(), DEBUG_LINE_FUNC);
                },
                *this);
        }

        template <typename T> inline T value::to_integer() const
        {
            return std::visit(
                [this](auto&& arg) -> T {
                    if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number>)
                        return rjson::to_integer<T>(arg);
                    else
                        throw value_type_mismatch("number", actual_type(), DEBUG_LINE_FUNC);
                },
                *this);
        }

        inline value::operator bool() const
        {
            return std::visit(
                [this](auto&& arg) -> bool {
                    if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, bool>)
                        return arg;
                    else
                        throw value_type_mismatch("bool", actual_type(), DEBUG_LINE_FUNC);
                },
                *this);
        }

        template <typename R> inline R value::get_or_default(R&& dflt) const
        {
            return std::visit(
                [this, &dflt](auto&& arg) -> R {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, null> || std::is_same_v<T, const_null>)
                        return dflt;
                    else
                        return *this;
                },
                *this);
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
                else if constexpr (std::is_same_v<T2, null> || std::is_same_v<T2, const_null>)
                    ;           // updating with null: do nothing
                else if constexpr (std::is_same_v<T1, const_null>)
                    throw merge_error(std::string{"cannot update ConstNull"});
                else
                    throw merge_error(std::string{"cannot merge two rjson values of different types"}); // : %" + to_string(*this) + "% and %" + to_string(arg2));
            };

            std::visit(visitor, *this, to_merge);
            return *this;
        }

        inline void value::remove_comments()
        {
            std::visit(
                [](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, object> || std::is_same_v<T, array>)
                        arg.remove_comments();
                },
                *this);
        }

        inline size_t value::max_index() const
        {
            return std::visit(
                [this](auto&& arg) -> size_t {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, object> || std::is_same_v<T, array>)
                        return arg.max_index();
                    else
                        throw value_type_mismatch("object or array", this->actual_type(), DEBUG_LINE_FUNC);
                },
                *this);
        }

        // ----------------------------------------------------------------------

        template <typename T> inline void copy(const value& source, T&& target)
        {
            std::visit(
                [&target, &source](auto&& arg) {
                    using TT = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<TT, object>)
                        throw value_type_mismatch("array", source.actual_type(), DEBUG_LINE_FUNC);
                    else if constexpr (std::is_same_v<TT, array>)
                        arg.copy_to(std::forward<T>(target));
                    else if constexpr (!std::is_same_v<TT, null> && !std::is_same_v<TT, const_null>)
                        throw value_type_mismatch("object or array", source.actual_type(), DEBUG_LINE_FUNC);
                },
                source);
        }

        template <typename T, typename F> inline void transform(const value& source, T&& target, F&& transformer)
        {
            static_assert(std::is_invocable_v<F, const object::value_type&> || std::is_invocable_v<F, const value&> || std::is_invocable_v<F, const value&, size_t>,
                          "rjson::transform: unsupported transformer signature");

            // std::is_const<typename std::remove_reference<const int&>::type>::value
            std::visit(
                [&target, &transformer, &source](auto&& arg) {
                    using TT = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<TT, object> && std::is_invocable_v<F, const object::value_type&>)
                        arg.transform_to(std::forward<T>(target), std::forward<F>(transformer));
                    else if constexpr (std::is_same_v<TT, array> && (std::is_invocable_v<F, const value&> || std::is_invocable_v<F, const value&, size_t>))
                        arg.transform_to(std::forward<T>(target), std::forward<F>(transformer));
                    else if constexpr (!std::is_same_v<TT, null> && !std::is_same_v<TT, const_null>) // do not remove, essential!
                        throw value_type_mismatch("object or array and corresponding transformer", source.actual_type(), DEBUG_LINE_FUNC);
                },
                source);
        }

        template <typename Value, typename F> inline void for_each(Value&& val, F&& func)
        {
            static_assert(std::is_invocable_v<F, const object::value_type&> || std::is_invocable_v<F, object::value_type&> || std::is_invocable_v<F, const value&> || std::is_invocable_v<F, value&> ||
                              std::is_invocable_v<F, const value&, size_t> || std::is_invocable_v<F, value&, size_t>,
                          "rjson::for_each: unsupported func signature");

            // std::is_const<typename std::remove_reference<const int&>::type>::value
            std::visit(
                [&func, &val](auto&& arg) {
                    using TT = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<TT, object> && (std::is_invocable_v<F, const object::value_type&> || std::is_invocable_v<F, object::value_type&>))
                        arg.for_each(func);
                    else if constexpr (std::is_same_v<TT, array> && (std::is_invocable_v<F, const value&> || std::is_invocable_v<F, value&> || std::is_invocable_v<F, const value&, size_t> ||
                                                                     std::is_invocable_v<F, value&, size_t>))
                        arg.for_each(func);
                    else if constexpr (!std::is_same_v<TT, null> && !std::is_same_v<TT, const_null>) // do not remove, essential!
                        throw value_type_mismatch("object or array and corresponding callback", val.actual_type(), DEBUG_LINE_FUNC);
                },
                std::forward<Value>(val));
        }

        template <typename Value, typename Func> inline auto find_if(Value&& value, Func&& func) // returns ConstNull if not found, Func: bool (value&&), throws if not array
        {
            return std::visit(
                [&func, &value](auto&& arg) -> Value& {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, array>)
                        return arg.find_if(std::forward<Func>(func));
                    // else if (func(*this))
                    //     return *this;
                    else
                        throw value_type_mismatch("array", value.actual_type(), DEBUG_LINE_FUNC);
                    // return ConstNull;
                },
                std::forward<Value>(value));
        }

        template <typename T> inline std::vector<T> as_vector(const value& source)
        {
            std::vector<T> result;
            copy(source, result);
            return result;
        }

        // ----------------------------------------------------------------------

        inline std::ostream& operator<<(std::ostream& out, const value& val) { return out << to_string(val); }

        // ----------------------------------------------------------------------

        inline void set_field_if_not_empty(value& target, const char* field_name, std::string source)
        {
            if (!source.empty())
                target[field_name] = source;
        }

        namespace detail
        {
            template <typename T> constexpr bool equal(T first, T second)
            {
                if constexpr (std::is_same_v<T, double>)
                    return float_equal(first, second);
                else
                    return first == second;
            }
        } // namespace detail

        template <typename T> inline void set_field_if_not_default(value& target, const char* field_name, T aValue, T aDefault)
        {
            if (!detail::equal(aValue, aDefault))
                target[field_name] = aValue;
        }

        inline void set_field_if_not_default(value& target, const char* field_name, double aValue, double aDefault, int precision)
        {
            if (!detail::equal(aValue, aDefault))
                target[field_name] = number(acmacs::to_string(aValue, precision));
        }

        template <typename S, typename Iterator> inline void set_array_field_if_not_empty(value& target, S key, Iterator first, Iterator last)
        {
            if (first != last) {
                auto& ar = target[key] = array{};
                for (; first != last; ++first)
                    ar.append(*first);
            }
        }

        template <typename S, typename Container> inline void set_array_field_if_not_empty(value& target, S key, Container&& container)
        {
            set_array_field_if_not_empty(target, key, std::begin(container), std::end(container));
        }

        // ----------------------------------------------------------------------

        template <typename T, typename = std::enable_if_t<!acmacs::sfinae::is_const_char_ptr_v<T>>>
            inline T get_or(const value& source, const char* field_name, T default_value)
        {
            if (source.is_null())
                return default_value;
            else if (const auto& val = source[field_name]; !val.is_null())
                return val;
            else
                return default_value;
        }

        inline std::string get_or(const value& source, const char* field_name, const char* default_value)
        {
            if (source.is_null())
                return default_value;
            else if (const auto& val = source[field_name]; !val.is_null())
                return val;
            else
                return default_value;
        }

        template <typename... Args> inline const value& one_of(const value& source, const char* field_name, Args... args)
        {
            if (const auto& val = source[field_name]; !val.is_null())
                return val;
            if constexpr (sizeof...(args) > 0)
                return one_of(source, args...);
            else
                return ConstNull;
        }

    } // namespace v2
} // namespace rjson

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
