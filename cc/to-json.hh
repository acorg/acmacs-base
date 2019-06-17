#pragma once

#include "acmacs-base/fmt.hh"
#include "acmacs-base/sfinae.hh"

// ----------------------------------------------------------------------

namespace to_json
{
    inline namespace v2
    {
        struct key_val;

        class error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

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

            template <typename T> friend json value(T&& val);
            friend json object();
            friend json array();
            friend json& append(json& target, const json& source);
            friend json& append(json& target, key_val&& source);
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

        inline json& append(json& target, const json& source)
        {
            const auto back = target.pop_back();
            switch (target.back()) {
              case '{':
              case '[':
                  break;
              default:
                  target.append(',');
                  break;
            }
            target.append(source.str());
            target.append(back);
            return target;
        }

        struct key_val
        {
            std::string_view key;
            const json& val;
        };

        inline json& append(json& target, key_val&& source)
        {
            const auto back = target.pop_back();
            switch (target.back()) {
              case '[':
                  throw error("cannot append key_val to array");
              case '{':
                  break;
              default:
                  target.append(',');
                  break;
            }
            target.append(value(source.key).str());
            target.append(':');
            target.append(source.val.str());
            target.append(back);
            return target;
        }

        template <typename T> inline json& operator<<(json& target, T&& source) { return append(target, std::forward<T>(source)); }

        // ----------------------------------------------------------------------

        inline json array() { return json("[]"); }
        // template <typename... Args> inline json array(Args&&... args) { return internal::array_append(std::string{}, std::forward<Args>(args)...); }

        template <typename Iterator, typename Transformer> inline json array(Iterator first, Iterator last, Transformer transformer)
        {
            auto js = array();
            for (; first != last; ++first)
                append(js, value(transformer(*first)));
            return js;
        }

        template <typename Iterator> inline json array(Iterator first, Iterator last)
        {
            auto js = array();
            for (; first != last; ++first)
                append(js, value(*first));
            return js;
        }

        // ----------------------------------------------------------------------

        inline json object() { return json("{}"); }
        inline json object(std::string key, const json& val) { auto js = object(); append(js, key_val{key, val}); return js; }

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
