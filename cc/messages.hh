#pragma once

#include <string_view>
#include <string>
#include <vector>

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace acmacs::messages::inline v1
{
    using key_t = std::string_view;
    using value_t = std::string;

    // ----------------------------------------------------------------------

    namespace key
    {
        constexpr static inline key_t unrecognized{"unrecognized"};
    }

    // ----------------------------------------------------------------------

    namespace detail
    {
        struct position_t
        {
            std::string filename{};
            size_t line_no{0};

            position_t() = default;
            position_t(std::string_view a_filename, size_t a_line_no) : filename{a_filename}, line_no{a_line_no} {}
        };
    } // namespace detail

    struct position_t : public detail::position_t
    {
        using detail::position_t::position_t;
    };

    struct code_position_t : public detail::position_t
    {
        using detail::position_t::position_t;
    };

    struct message_t
    {
        key_t key;
        std::string value;
        position_t source;
        code_position_t code;

        // message_t& operator=(message_t&&) = default;

        // parsing_message_t(const parsing_message_t&) = default;
        message_t(key_t a_key, std::string_view a_value = {}) : key{a_key}, value{a_value} {}
        message_t(key_t a_key, std::string_view a_value, const position_t& a_pos) : key{a_key}, value{a_value}, source{a_pos} {}
        message_t(key_t a_key, std::string_view a_value, const code_position_t& a_code) : key{a_key}, value{a_value}, code{a_code} {}
        message_t(key_t a_key, std::string_view a_value, const position_t& a_pos, const code_position_t& a_code) : key{a_key}, value{a_value}, source{a_pos}, code{a_code} {}
        // // parsing_message_t(std::string_view a_key, std::string_view a_value, std::string_view a_suppliment) : key{a_key}, value{a_value}, suppliment{a_suppliment} {}
        // parsing_message_t(std::string_view a_key = unrecognized) : key(a_key) {}
        // bool operator==(std::string_view a_key) const { return key == a_key; }
        // bool operator==(const parsing_message_t& rhs) const { return key == rhs.key && value == rhs.value; }
    };

    using messages_t = std::vector<message_t>;

    void move(messages_t& target, messages_t&& from);
    void move_and_add_source(messages_t& target, messages_t&& from, const position_t& source);
    void report_by_type(messages_t& messages);

    // ----------------------------------------------------------------------

    using iter_t = typename messages_t::const_iterator;
    using index_entry_t = std::pair<iter_t, iter_t>;
    using index_t = std::vector<index_entry_t>;

    index_t make_index(messages_t& messages);
    void report(const index_t& index);
    index_entry_t find(key_t key, const index_t& index);

} // namespace acmacs::messages::inline v1

// ----------------------------------------------------------------------

// template <> struct fmt::formatter<acmacs::messages::v1::detail::position_t> : public fmt::formatter<acmacs::fmt_default_formatter>
// {
//     template <typename FormatContext> auto format(const acmacs::messages::v1::detail::position_t& pos, FormatContext& ctx)
//     {
//         if (!pos.filename.empty())
//             format_to(ctx.out(), " @@ {}:{}", pos.filename, pos.line_no);
//         return ctx.out();
//     }
// };

// ----------------------------------------------------------------------

#define MESSAGE_CODE_POSITION acmacs::messages::v1::code_position_t{__FILE__, __LINE__}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
