#pragma once

#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <map>

// ----------------------------------------------------------------------

namespace rjson2
{
    class value;
    class array;
    class object;

    enum class json_pp_emacs_indent { no, yes };

      // --------------------------------------------------

    class null
    {
     public:
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

        void remove_comments();

     private:
        std::map<std::string, value> content_;

    }; // class object

      // --------------------------------------------------

    class array
    {
     public:
        array() = default;

        void remove_comments();

     private:
        std::vector<value> content_;

    }; // class array

      // --------------------------------------------------

    template <typename N> class number : public std::variant<N, std::string>
    {
     public:
        using base = std::variant<N, std::string>;
        number(std::string_view src) : base(std::string(src)) {}
        number(N src) : base(src) {}

    }; // class number<>

      // --------------------------------------------------

    using value_base = std::variant<null, object, array, std::string, number<long>, number<double>, bool>; // null must be the first alternative, it is the default value;

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

        void remove_comments()
        {
            std::visit(
                [](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, object> || std::is_same_v<T, array>)
                        arg.remove_comments();
                },
                *this);
        }

    }; // class value

      // --------------------------------------------------

    inline void array::remove_comments()
    {
        std::for_each(content_.begin(), content_.end(), [](auto& val) { val.remove_comments(); });
    }

    void object::remove_comments()
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

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
