#pragma once

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace injson
{
    inline namespace v1
    {
        class error : public std::runtime_error
        {
          public:
            template <typename S> error(size_t line_no, ssize_t column_no, S&& message) : std::runtime_error(fmt::format("injson error at {}:{}: {}", line_no, column_no, message)) {}
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

        template <typename Sink, typename Iter> void parse(Sink& sink, Iter first, Iter last)
        {
            using namespace fmt::literals;

            // const auto start = fist;
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
                        sink.json_object_start();
                        break;
                    case '}':
                        sink.json_object_end();
                        break;
                    case '[':
                        sink.json_array_start();
                        break;
                    case ']':
                        sink.json_array_end();
                        break;
                    case '"': {
                        const auto end = detail::read_string(first + 1, last, line_start, line_no); // may throw error on EOF
                        sink.json_string(first + 1, end);
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
                                sink.json_integer(first, end);
                                break;
                            case detail::number_is::real:
                                sink.json_real(first, end);
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
    } // namespace v1
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
