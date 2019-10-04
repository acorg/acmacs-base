#pragma once

#include <vector>
#include <map>
#include <cstdlib>

#include "acmacs-base/fmt.hh"
#include "acmacs-base/sfinae.hh"
#include "acmacs-base/to-string.hh"

// ----------------------------------------------------------------------

namespace to_json
{
    inline namespace v2
    {
        class json
        {
          private:
            static constexpr auto comma_after = [](auto iter, auto first) -> bool {
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

            static constexpr auto comma_before = [](auto iter) -> bool {
                switch (iter->back()) {
                    case ']':
                    case '}':
                    case ':':
                        return false;
                    default:
                        return true;
                }
            };

            static constexpr auto indent_after = [](auto iter, auto first) -> bool {
                if (iter == first)
                    return false;
                switch (std::prev(iter)->back()) {
                    case '[':
                    case '{':
                        return true;
                    default:
                        return false;
                }
            };

            static constexpr auto unindent_before = [](auto iter) -> bool {
                return iter->size() == 1 && (iter->front() == ']' || iter->front() == '}');
            };

          public:
            enum class compact_output { no, yes };
            enum class embed_space { no, yes };
            enum class escape_double_quotes { no, yes };

            std::string compact(embed_space space = embed_space::no) const
            {
                const std::string comma(space == embed_space::yes ? ", " : ",");
                fmt::memory_buffer out;
                for (auto chunk = data_.begin(); chunk != data_.end(); ++chunk) {
                    if (comma_after(chunk, data_.begin()) && comma_before(chunk))
                        fmt::format_to(out, "{}{}", comma, *chunk);
                    else if (space == embed_space::yes && chunk != data_.begin() && std::prev(chunk)->back() == ':')
                        fmt::format_to(out, " {}", *chunk);
                    else
                        fmt::format_to(out, "{}", *chunk);
                }
                return fmt::to_string(out);
            }

            std::string pretty(size_t indent) const
            {
                fmt::memory_buffer out;
                size_t current_indent = 0;
                for (auto chunk = data_.begin(); chunk != data_.end(); ++chunk) {
                    if (comma_after(chunk, data_.begin()) && comma_before(chunk)) {
                        fmt::format_to(out, ",\n{: >{}s}{}", "", current_indent, *chunk);
                    }
                    else {
                        if (const auto ia = indent_after(chunk, data_.begin()), ub = unindent_before(chunk); ia && !ub) {
                            current_indent += indent;
                            fmt::format_to(out, "\n{: >{}s}{}", "", current_indent, *chunk);
                        }
                        else if (!ia && ub) {
                            current_indent -= indent;
                            fmt::format_to(out, "\n{: >{}s}{}", "", current_indent, *chunk);
                        }
                        else if ((ia && ub) || chunk == data_.begin())
                            fmt::format_to(out, "{}", *chunk);
                        else if (chunk->front() == ':')
                            fmt::format_to(out, "{}", *chunk);
                        else
                            fmt::format_to(out, " {}", *chunk);
                    }
                }
                return fmt::to_string(out);
            }

            void move_before_end(json&& value) { std::move(value.data_.begin(), value.data_.end(), std::inserter(data_, std::prev(data_.end()))); }

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

            void make_compact()
            {
                data_[0] = compact(embed_space::yes);
                data_.erase(std::next(data_.begin()), data_.end());
            }

        }; // class json

        class val : public json
        {
          public:
            template <typename T> inline val(T&& a_val, escape_double_quotes esc = escape_double_quotes::no)
            {
                if constexpr (acmacs::sfinae::is_string_v<T>) {
                    if (esc == escape_double_quotes::yes)
                        push_back(fmt::format("\"{}\"", escape(a_val)));
                    else
                        push_back(fmt::format("\"{}\"", std::forward<T>(a_val)));
                }
                else if constexpr (std::numeric_limits<std::decay_t<T>>::is_integer || std::is_floating_point_v<std::decay_t<T>>)
                    push_back(fmt::format("{}", std::forward<T>(a_val)));
                else if constexpr (acmacs::sfinae::decay_equiv_v<T, bool>)
                    push_back(a_val ? "true" : "false");
                else
                    static_assert(std::is_same_v<int, std::decay_t<T>>, "invalid arg type for to_json::val");
            }

            static inline std::string escape(std::string_view str)
            {
                std::string result(static_cast<size_t>(str.size() * 1.2), ' ');
                auto output = std::begin(result);
                for (auto input = std::begin(str); input != std::end(str); ++input, ++output) {
                    switch (*input) {
                      case '"':
                          *output++ = '\\';
                          *output = *input;
                          break;
                      case '\n':
                          *output++ = '\\';
                          *output = 'n';
                          break;
                      case '\t':
                          *output = ' '; // old seqdb reader cannot read tabs
                          break;
                      default:
                          *output = *input;
                          break;
                    }
                }
                return result;
            }
        };

        class raw : public json
        {
          public:
            raw(std::string_view data) { push_back(data); }
            raw(std::string&& data) { push_back(std::move(data)); }
        };

        class key_val : public json
        {
          public:
            template <typename T> key_val(std::string_view key, T&& value, escape_double_quotes esc = escape_double_quotes::no)
            {
                move(val(key, esc));
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

            template <typename Iterator, typename Transformer, typename = acmacs::sfinae::iterator_t<Iterator>>
            array(Iterator first, Iterator last, Transformer transformer, compact_output co = compact_output::no, escape_double_quotes esc = escape_double_quotes::no)
                : array()
            {
                for (; first != last; ++first) {
                    auto value = transformer(*first);
                    if constexpr (std::is_convertible_v<std::decay_t<decltype(value)>, json>)
                        move_before_end(std::move(value));
                    else
                        move_before_end(val(std::move(value), esc));
                }
                if (co == compact_output::yes)
                    make_compact();
            }

            template <typename Iterator, typename = acmacs::sfinae::iterator_t<Iterator>> array(Iterator first, Iterator last, compact_output co = compact_output::no, escape_double_quotes esc = escape_double_quotes::no)
                : array()
            {
                for (; first != last; ++first)
                    move_before_end(val(*first, esc));
                if (co == compact_output::yes)
                    make_compact();
            }

          private:
            template <typename Arg1, typename... Args> void append(Arg1&& arg1, Args&&... args)
            {
                if constexpr (std::is_same_v<std::decay_t<Arg1>, compact_output>) {
                    if (arg1 == compact_output::yes)
                        make_compact();
                }
                else if constexpr (std::is_convertible_v<std::decay_t<Arg1>, json>)
                    move_before_end(std::move(arg1));
                else
                    move_before_end(val(std::forward<Arg1>(arg1)));
                if constexpr (sizeof...(args) > 0)
                    append(std::forward<Args>(args)...);
            }

        };

        class object : public json
        {
          public:
            object() : json('{', '}') {}
            template <typename... Args> object(Args&&... args) : object() { append(std::forward<Args>(args)...); }

            bool empty() const { return data_.size() == 2; }

            template <typename K, typename V> static object from(const std::map<K, V>& src)
            {
                object result;
                for (const auto& [key, val] : src)
                    result.move_before_end(key_val{acmacs::to_string(key), val});
                return result;
            }

          private:
            template <typename Arg1, typename... Args> void append(Arg1&& arg1, Args&&... args)
            {
                if constexpr (std::is_same_v<std::decay_t<Arg1>, compact_output>) {
                    if (arg1 == compact_output::yes)
                        make_compact();
                }
                else {
                    static_assert(std::is_convertible_v<std::decay_t<Arg1>, key_val>, "invalid arg type for to_json::object, must be to_json::key_val");
                    move_before_end(std::move(arg1));
                    if constexpr (sizeof...(args) > 0)
                        append(std::forward<Args>(args)...);
                }
            }

            friend object& operator<<(object& target, key_val&& kv);
        };

        template <typename T> inline array& operator<<(array& target, T&& value)
        {
            if constexpr (std::is_convertible_v<std::decay_t<T>, json>)
                target.move_before_end(std::forward<T>(value));
            else
                target.move_before_end(val(std::forward<T>(value)));
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

// "{}" -> pretty(2)
// "{:4}" -> pretty(4)
// "{:0}" -> compact()
// "{:4c}" -> compact()

template <typename T> struct fmt::formatter<T, std::enable_if_t<std::is_base_of<to_json::json, T>::value, char>> : fmt::formatter<std::string>
{
    template <typename ParseContext> auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it == ':')
            ++it;
        if (it != ctx.end() && *it != '}') {
            char* end;
            indent_ = std::strtoul(&*it, &end, 10);
            it = std::next(it, end - &*it);
        }
        if (*it == 'c') {
            indent_ = 0;        // compact anyway
            ++it;
        }
        while (it != ctx.end() && *it != '}')
            ++it;
        return it;
    }

    template <typename FormatCtx> auto format(const to_json::json& js, FormatCtx& ctx)
    {
        if (indent_ > 0)
            return fmt::formatter<std::string>::format(js.pretty(indent_), ctx);
        else
            return fmt::formatter<std::string>::format(js.compact(), ctx);
    }

    size_t indent_ = 2;
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
