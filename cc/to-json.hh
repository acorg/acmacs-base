#pragma once

#include <vector>

#include "acmacs-base/fmt.hh"
#include "acmacs-base/sfinae.hh"

// ----------------------------------------------------------------------

namespace to_json
{
    namespace v2
    {
        namespace detail
        {
            class base;
            class key_value;
        } // namespace detail

        namespace x
        {
            inline detail::key_value key_val(std::string key, detail::base&& value);
        }

        // ----------------------------------------------------------------------

        // class array;
        // class object;
        // class key_val;

        class json
        {
          public:
            std::string compact() const
            {
                const auto comma_after = [](auto iter, auto first) -> bool {
                    if (iter == first)
                        return false;
                    switch (std::prev(iter)->back()) {
                      case '[':
                      case '{':
                      case ':':
                          return false;
                      default:
                          return true;
                    }
                };

                const auto comma_before = [](auto iter) -> bool {
                    switch (iter->back()) {
                      case ']':
                      case '}':
                      case ':':
                          return false;
                      default:
                          return true;
                    }
                };

                fmt::memory_buffer out;
                for (auto chunk = data_.begin(); chunk != data_.end(); ++chunk) {
                    if (comma_after(chunk, data_.begin()) && comma_before(chunk))
                          fmt::format_to(out, ",{}", *chunk);
                    else
                          fmt::format_to(out, "{}", *chunk);
                }
                return fmt::to_string(out);
            }

            std::string pretty() const
            {
                fmt::memory_buffer out;
                for (const auto& chunk : data_)
                    fmt::format_to(out, "{}\n", chunk);
                return fmt::to_string(out);
            }

          protected:
            using data_t = std::vector<std::string>;

            data_t data_;

            json() = default;
            json(char beg, char end)
            {
                push_back(beg);
                push_back(end);
            }

            void push_back(std::string_view str) { data_.emplace_back(str); }
            void push_back(const char* str) { data_.emplace_back(str); }
            void push_back(std::string&& str) { data_.push_back(std::move(str)); }
            void push_back(char c) { data_.push_back(std::string(1, c)); }
            void move(json&& value) { std::move(value.data_.begin(), value.data_.end(), std::back_inserter(data_)); }
            void move_before_end(json&& value) { std::move(value.data_.begin(), value.data_.end(), std::inserter(data_, std::prev(data_.end()))); }
        };

        class val : public json
        {
          public:
            template <typename T> inline val(T&& a_val)
            {
                if constexpr (acmacs::sfinae::is_string_v<T>)
                    push_back(fmt::format("\"{}\"", std::forward<T>(a_val)));
                else if constexpr (std::numeric_limits<std::decay_t<T>>::is_integer || std::is_floating_point_v<std::decay_t<T>>)
                    push_back(fmt::format("{}", std::forward<T>(a_val)));
                else if constexpr (acmacs::sfinae::decay_equiv_v<T, bool>)
                    push_back(a_val ? "true" : "false");
                else
                    static_assert(std::is_same<int, std::decay_t<T>>::value, "invalid arg type for formatted_value");
            }
        };

        class key_val : public json
        {
          public:
            template <typename T> key_val(std::string_view key, T&& value)
            {
                move(val(key));
                push_back(':');
                if constexpr (std::is_convertible_v<std::decay_t<T>, json>)
                    move(std::move(value));
                else
                    move(val(std::forward<T>(value)));
            }
        };

        class array : public json
        {
          public:
            array() : json('[', ']') {}
            template <typename... Args> array(Args&&... args) : array() { append(std::forward<Args>(args)...); }

          private:
            template <typename Arg1, typename... Args> void append(Arg1&& arg1, Args&&... args)
            {
                if constexpr (std::is_convertible_v<std::decay_t<Arg1>, json>)
                    move_before_end(std::move(arg1));
                else
                    move_before_end(val(std::forward<Arg1>(arg1)));
                if constexpr (sizeof...(args) > 0)
                    append(std::forward<Args>(args)...);
            }

            friend array& operator<<(array& target, val&& value);
        };

        class object : public json
        {
          public:
            object() : json('{', '}') {}

            friend object& operator<<(object& target, key_val&& kv);
        };

        inline array& operator<<(array& target, val&& value)
        {
            target.move_before_end(std::move(value));
            return target;
        }

        inline object& operator<<(object& target, key_val&& kv)
        {
            target.move_before_end(std::move(kv));
            return target;
        }

    } // namespace v2

} // namespace to_json

// ----------------------------------------------------------------------

template <> struct fmt::formatter<to_json::v2::object>
{
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }
    template <typename FormatContext> auto format(const to_json::v2::object& js, FormatContext& ctx) { return format_to(ctx.out(), "{}", js.compact()); }
};

template <> struct fmt::formatter<to_json::v2::array>
{
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }
    template <typename FormatContext> auto format(const to_json::v2::array& js, FormatContext& ctx) { return format_to(ctx.out(), "{}", js.compact()); }
};

// ****************************************************************************************************

namespace to_json
{
    namespace v1_5
    {
        class json;

        class error : public std::runtime_error
        {
          public:
            using std::runtime_error::runtime_error;
        };

        struct key_val
        {
            std::string_view key;
            const json& val;
        };

        class json
        {
          public:
            std::string str() const
            {
                fmt::memory_buffer out;
                for (const auto& chunk : data_)
                    fmt::format_to(out, "{}\n", chunk);
                return fmt::to_string(out);
            }

          private:
            std::vector<std::string> data_;

            json() = default;
            json(char beg, char end)
            {
                data_.emplace_back(1, beg);
                data_.emplace_back(1, end);
            }
            json(const char* data) { data_.emplace_back(data); }
            json(const std::string& data) { data_.emplace_back(data); }
            json(std::string&& data) { data_.emplace_back(std::move(data)); }

            char back() const { return data_.back().back(); }
            char pop_back()
            {
                const auto bk = back();
                data_.pop_back();
                return bk;
            }
            void push_back(char c) { data_.push_back(std::string(1, c)); }
            void push_back(const std::string& source) { data_.push_back(source); }

            void push_back(json&& source) { std::move(source.data_.begin(), source.data_.end(), std::back_inserter(data_)); }

            void push_back(const json& source) { std::copy(source.data_.begin(), source.data_.end(), std::back_inserter(data_)); }

            void append(json&& source) { std::move(source.data_.begin(), source.data_.end(), std::inserter(data_, std::prev(data_.end()))); }

            void append(const json& source) { std::copy(source.data_.begin(), source.data_.end(), std::inserter(data_, std::prev(data_.end()))); }

            void append(key_val&& source);

            template <typename T> friend json value(T&& val);
            friend json object();
            friend json array();

            template <typename T> friend inline json& operator<<(json& target, T&& source)
            {
                target.append(std::forward<T>(source));
                return target;
            }
        };

        // ----------------------------------------------------------------------

        namespace detail
        {
            template <typename T> inline std::string formatted_value(T&& val)
            {
                if constexpr (acmacs::sfinae::is_string_v<T>)
                    return fmt::format("\"{}\"", std::forward<T>(val));
                else if constexpr (std::numeric_limits<std::decay_t<T>>::is_integer || std::is_floating_point_v<std::decay_t<T>>)
                    return fmt::format("{}", std::forward<T>(val));
                else if constexpr (acmacs::sfinae::decay_equiv_v<T, bool>)
                    return std::string(val ? "true" : "false");
                else
                    static_assert(std::is_same<int, std::decay_t<T>>::value, "invalid arg type for formatted_value");
            }

        } // namespace detail

        template <typename T> inline json value(T&& val)
        {
            if constexpr (acmacs::sfinae::is_string_v<T> || std::numeric_limits<std::decay_t<T>>::is_integer || std::is_floating_point_v<std::decay_t<T>> || acmacs::sfinae::decay_equiv_v<T, bool>)
                return json(detail::formatted_value(std::forward<T>(val)));
            else if constexpr (acmacs::sfinae::decay_equiv_v<T, json>)
                return std::move(val);
            else
                static_assert(std::is_same<int, std::decay_t<T>>::value, "use std::move?");
        }

        inline void json::append(key_val&& source)
        {
            const auto bk = pop_back();
            data_.push_back(detail::formatted_value(source.key));
            push_back(':');
            push_back(source.val);
            push_back(bk);
        }

        // ----------------------------------------------------------------------

        inline json array() { return json('[', ']'); }

        template <typename Iterator, typename Transformer, typename = acmacs::sfinae::iterator_t<Iterator>> inline json array(Iterator first, Iterator last, Transformer transformer)
        {
            auto js = array();
            for (; first != last; ++first)
                js << value(transformer(*first));
            return js;
        }

        template <typename Iterator, typename = acmacs::sfinae::iterator_t<Iterator>> inline json array(Iterator first, Iterator last)
        {
            auto js = array();
            for (; first != last; ++first)
                js << value(*first);
            return js;
        }

        namespace detail
        {
            template <typename Arg1, typename... Args> inline void array_append(json& js, Arg1&& arg1, Args&&... args)
            {
                js << value(std::forward<Arg1>(arg1));
                if constexpr (sizeof...(args) > 0)
                    array_append(js, std::forward<Args>(args)...);
            }
        } // namespace detail

        template <typename... Args> inline json array(Args&&... args)
        {
            auto js = array();
            if constexpr (sizeof...(args) > 0)
                detail::array_append(js, std::forward<Args>(args)...);
            return js;
        }

        // ----------------------------------------------------------------------

        namespace detail
        {
            template <typename Arg1, typename... Args> inline void object_append(json& js, Arg1&& arg1, Args&&... args)
            {
                js << std::forward<Arg1>(arg1);
                if constexpr (sizeof...(args) > 0)
                    object_append(js, std::forward<Args>(args)...);
            }
        } // namespace detail

        inline json object() { return json('{', '}'); }
        inline json object(std::string key, json&& val)
        {
            auto js = object();
            js << key_val{key, val};
            return js;
        }

        template <typename... Args> inline json object(Args&&... args)
        {
            auto js = object();
            if constexpr (sizeof...(args) > 0)
                detail::object_append(js, std::forward<Args>(args)...);
            return js;
        }

    } // namespace v1_5
} // namespace to_json

// ----------------------------------------------------------------------

template <> struct fmt::formatter<to_json::v1_5::json>
{
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }
    template <typename FormatContext> auto format(const to_json::v1_5::json& js, FormatContext& ctx) { return format_to(ctx.out(), "{}", js.str()); }
};


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
