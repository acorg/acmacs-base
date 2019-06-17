#pragma once

#include "acmacs-base/fmt.hh"
#include "acmacs-base/sfinae.hh"

// ----------------------------------------------------------------------

namespace to_json
{
    inline namespace v2
    {
        class json;

        class error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

        struct key_val
        {
            std::string_view key;
            const json& val;
        };

        class json
        {
          public:
            const std::string& str() const { return buffer_; }
            // json& append(std::string data) { const auto end = buffer_.back(); buffer_.pop_back(); buffer_.append(data); return *this; }
            // char back() const { return buffer_.back(); }
            // char last() const { return buffer_[buffer_.size() - 2]; }


          private:
            std::string buffer_;

            json() = default;
            json(const char* data) : buffer_{data} {}
            json(const std::string& data) : buffer_{data} {}
            json(std::string&& data) : buffer_{std::move(data)} {}

            char back() const { return buffer_.back(); }
            char pop_back() { const auto back = buffer_.back(); buffer_.pop_back(); return back; }
            void append(char c) { buffer_.append(1, c); }
            void append(const std::string& source) { buffer_.append(source); }

           void append(const json& source)
            {
                const auto bk = pop_back();
                switch (back()) {
                    case '{':
                    case '[':
                        break;
                    default:
                        append(',');
                        break;
                }
                append(source.str());
                append(bk);
            }

            void append(key_val&& source);

            template <typename T> friend json value(T&& val);
            friend json object();
            friend json array();

            template <typename T> friend inline json& operator<<(json& target, T&& source) { target.append(std::forward<T>(source)); return target; }
        };

        // ----------------------------------------------------------------------

        template <typename T> inline json value(T&& val)
        {
            if constexpr (acmacs::sfinae::is_string_v<T>)
                return json(fmt::format("\"{}\"", std::forward<T>(val)));
            else if constexpr (std::numeric_limits<std::decay_t<T>>::is_integer || std::is_floating_point_v<std::decay_t<T>>)
                return json(fmt::format("{}", std::forward<T>(val)));
            else if constexpr (acmacs::sfinae::decay_equiv_v<T, bool>)
                return json(val ? "true" : "false");
            else if constexpr (acmacs::sfinae::decay_equiv_v<T, json>)
                return std::move(val);
            else
                static_assert(std::is_same<int, std::decay_t<T>>::value, "use std::move?");
        }

        inline void json::append(key_val&& source)
            {
                const auto bk = pop_back();
                switch (back()) {
                    case '[':
                        throw error("cannot append key_val to array");
                    case '{':
                        break;
                    default:
                        append(',');
                        break;
                }
                append(value(source.key).str());
                append(':');
                append(source.val.str());
                append(bk);
            }

        // ----------------------------------------------------------------------

        inline json array() { return json("[]"); }

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

        inline json object() { return json("{}"); }
        inline json object(std::string key, const json& val) { auto js = object(); js << key_val{key, val}; return js; }

        template <typename... Args> inline json object(Args&&... args)
        {
            auto js = object();
            if constexpr (sizeof...(args) > 0)
                detail::object_append(js, std::forward<Args>(args)...);
            return js;
        }

    } // namespace v1
} // namespace to_json

// ----------------------------------------------------------------------

template <> struct fmt::formatter<to_json::v2::json>
{
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }
    template <typename FormatContext> auto format(const to_json::v2::json& js, FormatContext& ctx) { return format_to(ctx.out(), "{}", js.str()); }
};


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
