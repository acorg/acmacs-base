#pragma once

// ----------------------------------------------------------------------

// /usr/local/Cellar/llvm/10.0.0_3/include/c++/v1/__config
// https://isocpp.org/std/standing-documents/sd-6-sg10-feature-test-recommendations#__cpp_range_based_for
#if !defined(__cpp_impl_three_way_comparison) || __cpp_impl_three_way_comparison < 201907L
#error rjson::v3 requires <=> support
#endif

#if !defined(__cpp_concepts) || __cpp_concepts < 201907L
#error rjson::v3 requires concepts support
#endif

// ----------------------------------------------------------------------

#include <stdexcept>
#include <variant>
#include <string_view>

#include "acmacs-base/debug.hh"
#include "acmacs-base/float.hh"
#include "acmacs-base/flat-map.hh"
#include "acmacs-base/string-from-chars.hh"

// ----------------------------------------------------------------------

namespace rjson::v3
{
    class error : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

    class value_type_mismatch : public error
    {
      public:
        value_type_mismatch(std::string_view requested_type, std::string_view actual_type, std::string_view source_ref={})
            : error{fmt::format("value type mismatch, requested: {}, stored: {}{}", requested_type, actual_type, source_ref)}
        {
        }
    };

    class parse_error : public error
    {
      public:
        parse_error(size_t line, size_t column, std::string_view message) : error{""}, message_{fmt::format("{}:{}: {}", line, column, message)} {}
        const char* what() const noexcept override { return message_.data(); }

      private:
        std::string message_;

    }; // class parse_error

    // ----------------------------------------------------------------------

    class value;

    namespace detail
    {
        class null
        {
          public:
            template <typename Output> Output to() const
            {
                throw value_type_mismatch{typeid(Output).name(), "null"};
            }
        };

        class object
        {
          public:
            object() = default;

            constexpr bool empty() const noexcept { return true; }
            constexpr size_t size() const noexcept { return 0; }

            void insert(std::string_view aKey, value&& aValue);

            template <typename Output> Output to() const
            {
                throw value_type_mismatch{typeid(Output).name(), "object"};
            }

          private:
            using content_t = acmacs::map_with_unique_keys_t<std::string_view, value>;
            content_t content_;
        };

        class array
        {
          public:
            array() = default;

            constexpr bool empty() const noexcept { return true; }
            constexpr size_t size() const noexcept { return 0; }

            void append(value&& aValue);

            template <typename Output> Output to() const
            {
                throw value_type_mismatch{typeid(Output).name(), "array"};
            }

          private:
            std::vector<value> content_;
        };

        class simple
        {
          public:
            constexpr simple(std::string_view content) : content_{content} {}

            constexpr std::string_view _content() const noexcept { return content_; }

          private:
            std::string_view content_;
        };

        class string : public simple
        {
          public:
            constexpr bool empty() const noexcept { return _content().empty(); }
            constexpr size_t size() const noexcept { return _content().size(); }

            template <typename Output> Output to() const
            {
                if constexpr (std::is_constructible_v<Output, decltype(_content())>)
                    return Output{_content()};
                else
                    throw value_type_mismatch{typeid(Output).name(), fmt::format("string{{\"{}\"}}", _content())};
            }
        };

        class number : public simple
        {
          public:
            template <typename Output> Output to() const
            {
                if constexpr (std::is_floating_point_v<Output>) {
                    if (const auto val = acmacs::string::from_chars<Output>(_content()); !float_equal(val, std::numeric_limits<Output>::max()))
                        return val;
                    else
                        throw value_type_mismatch{typeid(Output).name(), fmt::format("number{{{}}}", _content())};
                }
                else if constexpr (std::is_arithmetic_v<Output> && !std::is_same_v<Output, bool>) {
                    if (const auto val = acmacs::string::from_chars<Output>(_content()); val != std::numeric_limits<Output>::max())
                        return val;
                    else
                        throw value_type_mismatch{typeid(Output).name(), fmt::format("number{{{}}}", _content())};
                }
                else
                    throw value_type_mismatch{typeid(Output).name(), fmt::format("number{{{}}}", _content())};
            }
        };

        class boolean
        {
          public:
            constexpr boolean(bool content) : content_{content} {}
            constexpr explicit operator bool() const noexcept { return content_; }

            template <typename Output> Output to() const
            {
                if constexpr (std::is_constructible_v<Output, bool> && !std::is_floating_point_v<Output>)
                    return Output{content_};
                else
                    throw value_type_mismatch{typeid(Output).name(), fmt::format("boolean{{{}}}", content_)};
            }

          private:
            bool content_;
        };

    } // namespace detail

    // ----------------------------------------------------------------------

    class value_read;

    class value
    {
      public:
        value() = default;
        value(value&&) = default;
        value& operator=(value&&) = default;
        constexpr value(detail::string&& src) : value_{std::move(src)} {}
        constexpr value(detail::number&& src) : value_{std::move(src)} {}
        constexpr value(detail::null&& src) : value_{std::move(src)} {}
        constexpr value(detail::boolean&& src) : value_{std::move(src)} {}
        constexpr value(detail::object&& src) : value_{std::move(src)} {}
        constexpr value(detail::array&& src) : value_{std::move(src)} {}

        virtual ~value();

        bool is_null() const noexcept;
        bool is_object() const noexcept;
        bool is_array() const noexcept;
        bool is_string() const noexcept;
        bool is_number() const noexcept;
        bool is_bool() const noexcept;
        std::string actual_type() const noexcept;

        template <typename Output> Output to() const; // throws value_type_mismatch

        bool empty() const noexcept;
        size_t size() const noexcept; // returns 0 if neither array nor object nor string

        std::string_view _content() const noexcept;

      private:
        using value_base = std::variant<detail::null, detail::object, detail::array, detail::string, detail::number, detail::boolean>; // null must be the first alternative, it is the default value;

        value_base value_;

        friend value_read parse(std::string&& data);

    }; // class value

    // ----------------------------------------------------------------------

    class value_read : public value
    {
      private:
        value_read(std::string&& buf) : buffer_{std::move(buf)} {}
        value& operator=(value&& val)
        {
            value::operator=(std::move(val));
            return *this;
        } // buffer_ untouched
        const std::string buffer_;

        friend value_read parse(std::string&& data);
    };

    // ======================================================================

    value_read parse_string(std::string_view data);
    value_read parse_file(std::string_view filename);

    // ======================================================================

    inline std::string value::actual_type() const noexcept
    {
        return std::visit([]<typename Content>(Content&& arg) -> std::string { return typeid(arg).name(); }, value_);
    }

    template <typename Output> inline Output value::to() const
    {
        return std::visit([]<typename Content>(Content&& arg) { return arg.template to<Output>(); }, value_);
    }

    inline std::string_view value::_content() const noexcept
    {
        return std::visit(
            []<typename Content>(Content&& arg) -> std::string_view {
                using T = std::decay_t<Content>;
                if constexpr (std::is_same_v<T, detail::string> || std::is_same_v<T, detail::number>)
                    return arg._content();
                else
                    return {};
            },
            value_);
    }

    inline bool value::empty() const noexcept
    {
        return std::visit(
            []<typename Content>(Content&& arg) {
                using T = std::decay_t<Content>;
                if constexpr (std::is_same_v<T, detail::object> || std::is_same_v<T, detail::array> || std::is_same_v<T, detail::string>)
                    return arg.empty();
                else if (std::is_same_v<T, detail::null>)
                    return true;
                else
                    return false;
            },
            value_);
    }

    inline size_t value::size() const noexcept
    {
        return std::visit(
            []<typename Content>(Content&& arg) -> size_t {
                using T = std::decay_t<Content>;
                if constexpr (std::is_same_v<T, detail::object> || std::is_same_v<T, detail::array> || std::is_same_v<T, detail::string>)
                    return arg.size();
                else
                    return 0;
            },
            value_);
    }

    inline bool value::is_null() const noexcept
    {
        return std::visit(
            []<typename Content>(Content&&) {
                if constexpr (std::is_same_v<std::decay_t<Content>, detail::null>)
                    return true;
                else
                    return false;
            },
            value_);
    }

    inline bool value::is_object() const noexcept
    {
        return std::visit(
            []<typename Content>(Content&&) {
                if constexpr (std::is_same_v<std::decay_t<Content>, detail::object>)
                    return true;
                else
                    return false;
            },
            value_);
    }

    inline bool value::is_array() const noexcept
    {
        return std::visit(
            []<typename Content>(Content&&) {
                if constexpr (std::is_same_v<std::decay_t<Content>, detail::array>)
                    return true;
                else
                    return false;
            },
            value_);
    }

    inline bool value::is_string() const noexcept
    {
        return std::visit(
            []<typename Content>(Content&&) {
                if constexpr (std::is_same_v<std::decay_t<Content>, detail::string>)
                    return true;
                else
                    return false;
            },
            value_);
    }

    inline bool value::is_number() const noexcept
    {
        return std::visit(
            []<typename Content>(Content&&) {
                if constexpr (std::is_same_v<std::decay_t<Content>, detail::number>)
                    return true;
                else
                    return false;
            },
            value_);
    }

    inline bool value::is_bool() const noexcept
    {
        return std::visit(
            []<typename Content>(Content&&) {
                if constexpr (std::is_same_v<std::decay_t<Content>, detail::boolean>)
                    return true;
                else
                    return false;
            },
            value_);
    }

    // ----------------------------------------------------------------------

    inline void detail::object::insert(std::string_view aKey, value&& aValue) { content_.emplace(aKey, std::move(aValue)); }

    inline void detail::array::append(value&& aValue) { content_.push_back(std::move(aValue)); }

} // namespace rjson::v3

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
