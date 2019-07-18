#pragma once

#include <stack>
#include <memory>

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace in_json
{
    inline namespace v1
    {
        class error : public std::runtime_error
        {
          public:
            template <typename S> error(size_t line_no, ssize_t column_no, S&& message) : std::runtime_error(fmt::format("in_json error at {}:{}: {}", line_no, column_no, message)) {}
        };

        class parse_error : public std::runtime_error
        {
          public:
            parse_error(std::string_view m1) : std::runtime_error{fmt::format("in_json parsing error: {}", m1)} {}
            parse_error(std::string_view m1, std::string_view m2) : parse_error(fmt::format("{}{}", m1, m2)) {}
        };

        namespace detail
        {
            template <typename Iter> Iter read_string(Iter first, Iter last, Iter& line_start, size_t& line_no)
            {
                bool esc = false;
                for (; first != last; ++first) {
                    switch (*first) {
                        case '"':
                            if (!esc)
                                return first;
                            esc = false;
                            break;
                        case '\\':
                            esc = !esc;
                            break;
                        case '\n':
                            esc = false;
                            ++line_no;
                            line_start = first + 1;
                            break;
                        default:
                            esc = false;
                            break;
                    }
                }
                throw error(line_no, first - line_start, "unexpected EOF");
            }

            enum class number_is { integer, real };
            template <typename Iter> std::pair<Iter, number_is> read_number(Iter first, Iter last)
            {
                number_is nis = number_is::integer;
                for (; first != last; ++first) {
                    switch (*first) {
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            break;
                        case '.':
                        case '-':
                        case 'e':
                        case 'E':
                            nis = number_is::real;
                            break;
                        default:
                            return {first, nis};
                    }
                }
                throw error(0, 0, "read_number internal");
            }
        } // namespace detail

        // ----------------------------------------------------------------------

        template <typename Sink, typename Iter> void parse(Sink& sink, Iter first, Iter last)
        {
            using namespace fmt::literals;

            auto line_start = first;
            size_t line_no = 1;
            while (first != last) {
                switch (*first) {
                    case ' ':
                    case ',':
                    case ':':
                        break;
                    case '\n':
                        line_start = first + 1;
                        ++line_no;
                        break;
                    case '{':
                        sink.injson_object_start();
                        break;
                    case '}':
                        sink.injson_object_end();
                        break;
                    case '[':
                        sink.injson_array_start();
                        break;
                    case ']':
                        sink.injson_array_end();
                        break;
                    case '"': {
                        const auto end = detail::read_string(first + 1, last, line_start, line_no); // may throw error on EOF
                        sink.injson_string(first + 1, end);
                        first = end; // end points to terminating double-quotes
                    } break;
                    case '+':
                    case '-':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9': {
                        const auto [end, nis] = detail::read_number(first + 1, last);
                        switch (nis) {
                            case detail::number_is::integer:
                                sink.injson_integer(first, end);
                                break;
                            case detail::number_is::real:
                                sink.injson_real(first, end);
                                break;
                        }
                        first = end - 1; // incremented after switch
                    } break;
                    default:
                        throw error(line_no, first - line_start, "unexpected '{}'"_format(*first));
                }
                ++first;
            }
        }

        // ----------------------------------------------------------------------

        class stack_entry
        {
          public:
            stack_entry() = default;
            stack_entry(const stack_entry&) = default;
            virtual ~stack_entry() = default;
            virtual const char* injson_name() = 0;
            virtual std::unique_ptr<stack_entry> injson_put_object() { throw in_json::parse_error(fmt::format("{}: unexpected subobject for key \"{}\"", injson_name(), key_)); }
            virtual void injson_put_key(std::string_view data) { key_ = data; }
            virtual void injson_put_string(std::string_view data) { throw parse_error(fmt::format("{}: unexpected string \"{}\" for key \"{}\"", injson_name(), data, key_)); }
            virtual void injson_put_integer(std::string_view data) { throw parse_error(fmt::format("{}: unexpected integer {} for key \"{}\"", injson_name(), data, key_)); }
            virtual void injson_put_real(std::string_view data) { throw parse_error(fmt::format("{}: unexpected real {} for key \"{}\"", injson_name(), data, key_)); }
            virtual void injson_put_array() { throw parse_error(fmt::format("{}: unexpected array for key \"{}\"", injson_name(), key_)); }
            virtual void injson_pop_array() {} //  throw parse_error("stack_entry::injson_pop_array");
            bool key_empty() const { return key_.empty(); }

          protected:
            std::string_view key_{};

            void reset_key() { key_ = std::string_view{}; }
        };

        template <typename TargetContainer, typename ToplevelStackEntry> class object_sink
        {
          public:
            object_sink(TargetContainer& target) : target_{target} {}

            void injson_object_start()
            {
                if (stack_.empty())
                    stack_.push(std::make_unique<ToplevelStackEntry>(target_));
                else
                    stack_.push(stack_.top()->injson_put_object());
            }

            void injson_object_end() { stack_.pop(); }
            void injson_array_start() { stack_.top()->injson_put_array(); }
            void injson_array_end() { stack_.top()->injson_pop_array(); }
            template <typename Iter> void injson_string(Iter first, Iter last)
            {
                const std::string_view data{&*first, static_cast<size_t>(last - first)};
                if (auto& tar = *stack_.top(); tar.key_empty())
                    tar.injson_put_key(data);
                else
                    tar.injson_put_string(data);
            }
            template <typename Iter> void injson_integer(Iter first, Iter last) { stack_.top()->injson_put_integer({&*first, static_cast<size_t>(last - first)}); }
            template <typename Iter> void injson_real(Iter first, Iter last) { stack_.top()->injson_put_real({&*first, static_cast<size_t>(last - first)}); }

          private:
            TargetContainer& target_;
            std::stack<std::unique_ptr<stack_entry>> stack_;
        };

    } // namespace v1
} // namespace in_json

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
