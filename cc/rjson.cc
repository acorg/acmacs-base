#include <stack>
#include <memory>

#include "acmacs-base/rjson.hh"
#include "acmacs-base/read-file.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

rjson::v2::value rjson::v2::ConstNull{rjson::v2::const_null{}};

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

namespace rjson
{
    inline namespace v2
    {
        namespace parser_pop
        {
            class SymbolHandler;

            class StateTransitionNone
            {
            };
            class StateTransitionPop
            {
            };

            using HandlingResult = std::variant<StateTransitionNone, StateTransitionPop, std::unique_ptr<SymbolHandler>>;

            // --------------------------------------------------

            class Parser
            {
              public:
                Parser();

                void parse(std::string_view data);
                value result_move();
                void remove_emacs_indent();
                void remove_comments();

                constexpr auto pos() const noexcept { return pos_; }
                constexpr auto line() const noexcept { return line_; }
                constexpr auto column() const noexcept { return column_; }
                std::string_view data(size_t aBegin, size_t aEnd) const { return {source_.data() + aBegin, aEnd - aBegin}; }

                void back() noexcept
                {
                    --pos_;
                    if (--column_ == 0)
                        --line_;
                }
                constexpr void newline() noexcept
                {
                    ++line_;
                    column_ = 0;
                }

              private:
                std::string_view source_;
                size_t pos_ = 0, line_ = 1, column_ = 1;
                std::stack<std::unique_ptr<SymbolHandler>> handlers_;

                void pop();

            }; // class Parser

            // --------------------------------------------------

            class SymbolHandler
            {
              public:
                virtual ~SymbolHandler() = default;

                [[noreturn]] void unexpected(std::string_view::value_type aSymbol, Parser& aParser) const {
                    throw parse_error(aParser.line(), aParser.column(),
                                      std::string{"unexpected symbol: '"} + aSymbol + "' (" + acmacs::to_hex_string(static_cast<unsigned char>(aSymbol), acmacs::ShowBase, acmacs::Uppercase) + ")");
                }

                    [[noreturn]] void error(Parser& aParser, std::string&& aMessage) const
                {
                    throw parse_error(aParser.line(), aParser.column(), std::move(aMessage));
                }

                [[noreturn]] void internal_error(Parser& aParser) const { throw parse_error(aParser.line(), aParser.column(), "internal error"); }

                void newline(Parser& aParser) const
                {
                    aParser.newline();
                }

                virtual HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser)
                {
                    switch (aSymbol) {
                        case ' ':
                        case '\t':
                        case '\r':
                            break;
                        case '\n':
                            newline(aParser);
                            break;
                        default:
                            unexpected(aSymbol, aParser);
                    }
                    return StateTransitionNone{};
                }

                virtual value value_move() = 0; //{ return null{}; }
                virtual void subvalue(value&& /*aSubvalue*/, Parser& /*aParser*/) {}

            }; // class SymbolHandler

            // --------------------------------------------------

            class StringEscapeHandler : public SymbolHandler
            {
              public:
                HandlingResult handle(std::string_view::value_type /*aSymbol*/, Parser& /*aParser*/) override { return StateTransitionPop{}; }

                value value_move() override { return {}; }

            }; // class StringEscapeHandler

            // ----------------------------------------------------------------------

            class StringHandler : public SymbolHandler
            {
              public:
                StringHandler(Parser& aParser) : parser_{aParser}, begin_{aParser.pos() + 1}, end_{0} {}

                HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
                {
                    HandlingResult result = StateTransitionNone{};
                    switch (aSymbol) {
                        case '"':
                            result = StateTransitionPop{};
                            end_ = aParser.pos();
                            break;
                        case '\\':
                            result = std::make_unique<StringEscapeHandler>();
                            break;
                        case '\n':
                            newline(aParser);
                            break;
                        default:
                            break;
                    }
                    return result;
                }

                value value_move() override { return parser_.data(begin_, end_); }

              private:
                Parser& parser_;
                size_t begin_, end_;

            }; // class StringHandler

            // ----------------------------------------------------------------------

            class NumberHandler : public SymbolHandler
            {
              public:
                NumberHandler(Parser& aParser) : parser_{aParser}, begin_{aParser.pos()}, end_{0} {}

                HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
                {
                    HandlingResult result = StateTransitionNone{};
                    end_ = aParser.pos() + 1;
                    switch (aSymbol) {
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
                            sign_allowed_ = false;
                            break;
                        case '.':
                            if (exponent_)
                                unexpected(aSymbol, aParser);
                            // integer_ = false;
                            sign_allowed_ = false;
                            break;
                        case 'e':
                        case 'E':
                            // integer_ = false;
                            exponent_ = true;
                            sign_allowed_ = true;
                            break;
                        case '-':
                        case '+':
                            if (!sign_allowed_)
                                unexpected(aSymbol, aParser);
                            sign_allowed_ = false;
                            break;
                        default:
                            result = StateTransitionPop{};
                            end_ = aParser.pos();
                            aParser.back();
                            break;
                    }
                    return result;
                }

                value value_move() override { return number(std::string(parser_.data(begin_, end_))); }

              private:
                Parser& parser_;
                size_t begin_, end_;
                bool sign_allowed_ = true;
                bool exponent_ = false;
                // bool integer_ = true;

            }; // class NumberHandler

            // ----------------------------------------------------------------------

            class BoolNullHandler : public SymbolHandler
            {
              public:
                BoolNullHandler(const char* aExpected, value&& aValue) : expected_{aExpected}, value_{std::move(aValue)} {}

                HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
                {
                    HandlingResult result = StateTransitionNone{};
                    switch (aSymbol) {
                        case 'r':
                        case 'u':
                        case 'e':
                        case 'a':
                        case 'l':
                        case 's':
                            if (expected_[pos_] != aSymbol)
                                unexpected(aSymbol, aParser);
                            break;
                        default:
                            if (expected_[pos_] == 0) {
                                result = StateTransitionPop{};
                                aParser.back();
                            }
                            else {
                                result = SymbolHandler::handle(aSymbol, aParser);
                            }
                    }
                    ++pos_;
                    return result;
                }

                value value_move() override { return std::move(value_); }

              private:
                const char* expected_;
                value value_;

                size_t pos_ = 0;

            }; // class BoolNullHandler

            // ----------------------------------------------------------------------

            class ValueHandler : public SymbolHandler
            {
              public:
                HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override;

                void subvalue(value&& subvalue, Parser& /*aParser*/) override
                {
                    value_ = std::move(subvalue);
                    value_read_ = true;
                }

                value value_move() override { return std::move(value_); }
                auto& value_ref() { return value_; }

              protected:
                constexpr bool value_read() const { return value_read_; }

              private:
                bool value_read_ = false;
                value value_;

            }; // class ValueHandler

            // --------------------------------------------------

            class ObjectHandler : public SymbolHandler
            {
              private:
                enum class Expected { Key, KeyAfterComma, Value, Colon, Comma };

              public:
                ObjectHandler() {}

                HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
                {
                    HandlingResult result = StateTransitionNone{};
                    switch (aSymbol) {
                        case '}':
                            switch (expected_) {
                                case Expected::Key:
                                case Expected::Comma:
                                    result = StateTransitionPop{};
                                    break;
                                case Expected::KeyAfterComma:
                                    error(aParser, "unexpected " + std::string{aSymbol} + " -- did you forget to remove last comma?");
                                case Expected::Value:
                                case Expected::Colon:
                                    unexpected(aSymbol, aParser);
                            }
                            break;
                        case ',':
                            switch (expected_) {
                                case Expected::Comma:
                                    expected_ = Expected::KeyAfterComma;
                                    break;
                                case Expected::Key:
                                    error(aParser, "unexpected comma right after the beginning of an object");
                                case Expected::KeyAfterComma:
                                    error(aParser, "unexpected comma -- two successive commas?");
                                case Expected::Value:
                                    error(aParser, "unexpected comma after colon"); // never comes here (processed by ValueHandler)
                                case Expected::Colon:
                                    error(aParser, "unexpected comma, colon is expected there");
                            }
                            break;
                        case ':':
                            if (expected_ == Expected::Colon) {
                                expected_ = Expected::Value;
                                result = std::make_unique<ValueHandler>();
                            }
                            else
                                unexpected(aSymbol, aParser);
                            break;
                        case '"':
                            switch (expected_) {
                                case Expected::Key:
                                case Expected::KeyAfterComma:
                                case Expected::Value:
                                    result = std::make_unique<StringHandler>(aParser);
                                    break;
                                case Expected::Comma:
                                    error(aParser, "unexpected " + std::string{aSymbol} + " -- did you forget comma?");
                                case Expected::Colon:
                                    unexpected(aSymbol, aParser);
                            }
                            break;
                        default:
                            result = SymbolHandler::handle(aSymbol, aParser);
                            break;
                    }
                    return result;
                }

                value value_move() override { return std::move(value_); }

                void subvalue(value&& aSubvalue, Parser& aParser) override
                {
                    switch (expected_) {
                        case Expected::Key:
                        case Expected::KeyAfterComma:
                            key_ = std::move(aSubvalue);
                            expected_ = Expected::Colon;
                            break;
                        case Expected::Value:
                            value_.insert(std::move(key_), std::move(aSubvalue));
                            expected_ = Expected::Comma;
                            break;
                        case Expected::Comma:
                        case Expected::Colon:
                            internal_error(aParser);
                    }
                }

              private:
                object value_;
                value key_;
                Expected expected_ = Expected::Key;

            }; // class ObjectHandler

            // ----------------------------------------------------------------------

            class ArrayHandler : public SymbolHandler
            {
              private:
                enum class Expected { Value, ValueAfterComma, Comma };

              public:
                ArrayHandler() {}

                HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
                {
                    HandlingResult result = StateTransitionNone{};
                    switch (aSymbol) {
                        case ']':
                            switch (expected_) {
                                case Expected::Value:
                                case Expected::Comma:
                                    result = StateTransitionPop{};
                                    break;
                                case Expected::ValueAfterComma:
                                    error(aParser, "unexpected " + std::string{aSymbol} + " -- did you forget to remove last comma?");
                            }
                            break;
                        case ',':
                            switch (expected_) {
                                case Expected::Value:
                                    error(aParser, "unexpected comma right after the beginning of an array");
                                case Expected::Comma:
                                    expected_ = Expected::ValueAfterComma;
                                    break;
                                case Expected::ValueAfterComma:
                                    error(aParser, "unexpected comma -- two successive commas?");
                            }
                            break;
                        case '\n':
                            newline(aParser);
                            break;
                        case ' ':
                        case '\t':
                            break;
                        default:
                            switch (expected_) {
                                case Expected::Value:
                                case Expected::ValueAfterComma:
                                    result = std::make_unique<ValueHandler>();
                                    aParser.back();
                                    expected_ = Expected::Comma;
                                    break;
                                case Expected::Comma:
                                    error(aParser, "unexpected " + std::string{aSymbol} + " -- did you forget comma?");
                            }
                            break;
                    }
                    return result;
                }

                value value_move() override { return std::move(value_); }

                void subvalue(value&& aSubvalue, Parser& /*aParser*/) override { value_.append(std::move(aSubvalue)); }

              private:
                array value_;
                Expected expected_ = Expected::Value;

            }; // class ArrayHandler

            // ----------------------------------------------------------------------

            class ToplevelHandler : public ValueHandler
            {
              public:
                HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
                {
                    HandlingResult result;
                    if (value_read())
                        result = SymbolHandler::handle(aSymbol, aParser);
                    else
                        result = ValueHandler::handle(aSymbol, aParser);
                    return result;
                }

            }; // class ToplevelHandler

            // --------------------------------------------------

            HandlingResult ValueHandler::handle(std::string_view::value_type aSymbol, Parser& aParser)
            {
                HandlingResult result = StateTransitionNone{};
                if (value_read()) {
                    aParser.back();
                    result = StateTransitionPop{};
                }
                else {
                    switch (aSymbol) {
                        case '"':
                            result = std::make_unique<StringHandler>(aParser);
                            break;
                        case '-':
                        case '+':
                        case '.':
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
                            result = std::make_unique<NumberHandler>(aParser);
                            aParser.back();
                            break;
                        case 't':
                            result = std::make_unique<BoolNullHandler>("rue", true);
                            break;
                        case 'f':
                            result = std::make_unique<BoolNullHandler>("alse", false);
                            break;
                        case 'n':
                            result = std::make_unique<BoolNullHandler>("ull", null{});
                            break;
                        case '{':
                            result = std::make_unique<ObjectHandler>();
                            break;
                        case '[':
                            result = std::make_unique<ArrayHandler>();
                            break;
                        default:
                            result = SymbolHandler::handle(aSymbol, aParser);
                            break;
                    }
                }
                return result;
            }

            // --------------------------------------------------

            inline Parser::Parser() { handlers_.emplace(new ToplevelHandler); }

            inline void Parser::pop()
            {
                auto value{handlers_.top()->value_move()};
                handlers_.pop();
                handlers_.top()->subvalue(std::move(value), *this);
            }

            void Parser::parse(std::string_view data)
            {
                source_ = data;
                for (pos_ = 0; pos_ < source_.size(); ++pos_, ++column_) {
                    const auto symbol = source_[pos_];
                    auto handling_result = handlers_.top()->handle(symbol, *this);
                    std::visit(
                        [this](auto&& arg) {
                            using T = std::decay_t<decltype(arg)>;
                            if constexpr (std::is_same_v<T, std::unique_ptr<SymbolHandler>>) {
                                this->handlers_.push(std::forward<T>(arg));
                            }
                            else if constexpr (std::is_same_v<T, StateTransitionPop>) {
                                pop();
                            }
                        },
                        handling_result);
                }
                if (handlers_.size() > 1)
                    pop();
            }

            inline value Parser::result_move() { return handlers_.top()->value_move(); }

            inline void Parser::remove_emacs_indent()
            {
                auto& val = dynamic_cast<ToplevelHandler*>(handlers_.top().get())->value_ref();
                std::visit(
                    [](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, object>) {
                            arg.remove("_");
                        }
                    },
                    val);
            }

            inline void Parser::remove_comments() { dynamic_cast<ToplevelHandler*>(handlers_.top().get())->value_ref().remove_comments(); }

            // --------------------------------------------------

            template <typename S> inline value parse_string(S data, remove_comments rc)
            {
                Parser parser{};
                parser.parse(data);
                parser.remove_emacs_indent();
                if (rc == remove_comments::yes)
                    parser.remove_comments();
                return parser.result_move();
            }

        } // namespace parser_pop
    }     // namespace v2
} // namespace rjson

// ----------------------------------------------------------------------

rjson::v2::value rjson::v2::parse_string(std::string data, remove_comments rc)
{
    return rjson::v2::parser_pop::parse_string(data, rc);

} // rjson::v2::parse_string

// ----------------------------------------------------------------------

rjson::v2::value rjson::v2::parse_string(std::string_view data, remove_comments rc)
{
    return rjson::v2::parser_pop::parse_string(data, rc);

} // rjson::v2::parse_string

// ----------------------------------------------------------------------

rjson::v2::value rjson::v2::parse_string(const char* data, remove_comments rc)
{
    return rjson::v2::parser_pop::parse_string(data, rc);

} // rjson::v2::parse_string

// ----------------------------------------------------------------------

rjson::v2::value rjson::v2::parse_file(std::string filename, remove_comments rc)
{
    return parse_string(acmacs::file::read(filename), rc);

} // rjson::v2::parse_file

// ----------------------------------------------------------------------

std::string rjson::v2::to_string(const object& val, bool space_after_comma)
{
    std::string result(1, '{');
    size_t size_at_comma = 0;
    for (const auto& [key, val2] : val.content_) {
        // if (key != force_pp_key && key != no_pp_key) {
            result.append(1, '"');
            result.append(key);
            result.append("\":");
            if (space_after_comma)
                result.append(1, ' ');
            result.append(to_string(val2));
            size_at_comma = result.size() + 1;
            result.append(1, ',');
            if (space_after_comma)
                result.append(1, ' ');
        // }
    }
    if (result.back() == ',' || result.back() == ' ') {
        result.resize(size_at_comma);
        result.back() = '}';
    }
    else
        result.append(1, '}');
    return result;
}

// ----------------------------------------------------------------------

std::string rjson::v2::to_string(const array& val, bool space_after_comma)
{
    std::string result(1, '[');
    for (const auto& val2: val.content_) {
        result.append(to_string(val2));
        result.append(1, ',');
        if (space_after_comma)
            result.append(1, ' ');
    }
    if (space_after_comma && result.size() > 2 && result.back() == ' ') {
        result.resize(result.size() - 1);
        result.back() = ']';
    }
    else if (result.back() == ',')
        result.back() = ']';
    else
        result.append(1, ']');
    return result;
}

// ----------------------------------------------------------------------

namespace rjson
{
    inline namespace v2
    {
        enum class dive { no, yes };
        template <typename RJ> static inline bool is_simple(RJ&&, dive = dive::yes) { return true; }

        template <> inline bool is_simple(const object& val, dive a_dive)
        {
            return val.empty() || (a_dive == dive::yes && val.all_of([](const auto& kv) -> bool { return is_simple(kv.second, dive::no); }));
        }

        template <> inline bool is_simple(const array& val, dive a_dive)
        {
            return val.empty() || ((a_dive == dive::yes) && val.all_of([](const auto& v) -> bool { return is_simple(v, dive::no); }));
        }

        template <> inline bool is_simple(const value& val, dive a_dive)
        {
            return std::visit([a_dive](auto&& arg) -> bool { return is_simple(arg, a_dive); }, val);
        }

    } // namespace v2
} // namespace rjson

// ----------------------------------------------------------------------

std::string rjson::v2::pretty(const object& val, size_t indent, json_pp_emacs_indent emacs_indent, size_t prefix)
{
    if (is_simple(val))
        return to_string(val, true);

    std::string result(1, '{');
    if (emacs_indent == json_pp_emacs_indent::yes && indent) {
        result.append(indent - 1, ' ');
        result.append("\"_\": \"-*- js-indent-level: " + std::to_string(indent) + " -*-\",\n");
    }
    else {
        result.append(1, '\n');
    }
    size_t size_before_comma = result.size() - 1;
    result.append(prefix + indent, ' ');
    for (const auto& [key, val2] : val.content_) {
        result.append("\"" + key + "\": ");
        result.append(pretty(val2, indent, json_pp_emacs_indent::no, prefix + indent));
        size_before_comma = result.size();
        result.append(",\n");
        result.append(prefix + indent, ' ');
    }
    if (result.back() == ' ') {
        result.resize(size_before_comma);
        result.append(1, '\n');
        result.append(prefix, ' ');
        result.append(1, '}');
    }
    else
        result.append(1, '}');
    return result;

} // rjson::v2::pretty

// ----------------------------------------------------------------------

std::string rjson::v2::pretty(const array& val, size_t indent, json_pp_emacs_indent /*emacs_indent*/, size_t prefix)
{
    if (is_simple(val))
        return to_string(val, true);

    std::string result("[\n");
    size_t size_before_comma = 1;
    result.append(prefix + indent, ' ');
    for (const auto& val2: val.content_) {
        result.append(pretty(val2, indent, json_pp_emacs_indent::no, prefix + indent));
        size_before_comma = result.size();
        result.append(",\n");
        result.append(prefix + indent, ' ');
    }
    if (result.back() == ' ') {
        result.resize(size_before_comma);
        result.append(1, '\n');
        result.append(prefix, ' ');
    }
    result.append(1, ']');
    return result;

} // rjson::v2::pretty

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
