// POP rjson parser implementation
// upon parsing a value (obejct, array, string, etc.) its value moved to parent

#error Obsolete

#include <stack>
#include <memory>
#include <string_view>

#include "acmacs-base/string.hh"

#include "rjson-v2.hh"
#include "rjson-parser-pop-v1.hh"

// ----------------------------------------------------------------------

namespace rjson
{
    namespace v1
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

            using HandlingResultBase = std::variant<StateTransitionNone, StateTransitionPop, std::unique_ptr<SymbolHandler>>;

            class HandlingResult : public HandlingResultBase
            {
              public:
                using HandlingResultBase::HandlingResultBase;

            }; // class HandlingResult

            // ----------------------------------------------------------------------

            class Parser
            {
              public:
                Parser();

                void parse(std::string_view aJsonData);

                // value result() const;
                value result_move();
                void remove_emacs_indent();
                void remove_comments();
                inline size_t pos() const { return mPos; }
                inline size_t line() const { return mLine; }
                inline size_t column() const { return mColumn; }
                inline std::string_view::value_type previous() const
                {
                    if (mPos)
                        return mSource[mPos - 1];
                    throw parse_error(line(), pos(), "internal: no previous at the beginning of parsing");
                }
                inline std::string data(size_t aBegin, size_t aEnd) const { return {mSource.data() + aBegin, aEnd - aBegin}; }

                inline void back() noexcept
                {
                    // if (!mPos)
                    //     throw parse_error(line(), pos(), "internal: cannot back at the beginning of parsing");
                    --mPos;
                    --mColumn;
                    if (mColumn == 0)
                        --mLine;
                }

                inline void newline() noexcept
                {
                    ++mLine;
                    mColumn = 0;
                }

              private:
                std::string_view mSource;
                size_t mPos, mLine, mColumn;
                std::stack<std::unique_ptr<SymbolHandler>> mHandlers;

                void pop();

            }; // class Parser

            // ----------------------------------------------------------------------

            class SymbolHandler
            {
              public:
                virtual ~SymbolHandler(){}

                    [[noreturn]] inline void unexpected(std::string_view::value_type aSymbol, Parser& aParser) const
                {
                    throw parse_error(aParser.line(), aParser.column(),
                                             std::string{"unexpected symbol: '"} + aSymbol + "' (" + acmacs::to_hex_string(static_cast<unsigned char>(aSymbol), acmacs::ShowBase, acmacs::Uppercase) +
                                                 ")");
                }

                [[noreturn]] inline void error(Parser& aParser, std::string&& aMessage) const { throw parse_error(aParser.line(), aParser.column(), std::move(aMessage)); }

                [[noreturn]] inline void internal_error(Parser& aParser) const { throw parse_error(aParser.line(), aParser.column(), "internal error"); }

                inline void newline(Parser& aParser) const { aParser.newline(); }

                virtual inline HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser)
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

                // virtual inline value val() const { return null{}; }
                virtual value value_move() = 0; //{ return null{}; }
                virtual inline void subvalue(value&& /*aSubvalue*/, Parser& /*aParser*/) {}

            }; // class SymbolHandler

            // ----------------------------------------------------------------------

            class ValueHandler : public SymbolHandler
            {
              public:
                HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override;

                inline void subvalue(value&& aSubvalue, Parser& /*aParser*/) override
                {
                    mValue = std::move(aSubvalue);
                    mValueRead = true;
                }

                // inline value val() const override { return mValue; }
                inline value value_move() override { return std::move(mValue); }
                inline value& val() { return mValue; }

              protected:
                inline bool value_read() const { return mValueRead; }

              private:
                bool mValueRead = false;
                class value mValue;

            }; // class ValueHandler

            // ----------------------------------------------------------------------

            class StringEscapeHandler : public SymbolHandler
            {
              public:
                inline HandlingResult handle(std::string_view::value_type /*aSymbol*/, Parser& /*aParser*/) override { return StateTransitionPop{}; }

                inline value value_move() override { return {}; }

            }; // class StringEscapeHandler

            // ----------------------------------------------------------------------

            class StringHandler : public SymbolHandler
            {
              public:
                inline StringHandler(Parser& aParser) : mParser{aParser}, mBegin{aParser.pos() + 1}, mEnd{0} {}

                inline HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
                {
                    HandlingResult result = StateTransitionNone{};
                    switch (aSymbol) {
                        case '"':
                            result = StateTransitionPop{};
                            mEnd = aParser.pos();
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

                // inline value val() const override
                //     {
                //         return string{mParser.data(mBegin, mEnd)};
                //     }

                inline value value_move() override { return string{mParser.data(mBegin, mEnd)}; }

              private:
                Parser& mParser;
                size_t mBegin, mEnd;

            }; // class StringHandler

            // ----------------------------------------------------------------------

            class NumberHandler : public SymbolHandler
            {
              public:
                inline NumberHandler(Parser& aParser) : mParser{aParser}, mBegin{aParser.pos()}, mEnd{0} {}

                inline HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
                {
                    HandlingResult result = StateTransitionNone{};
                    mEnd = aParser.pos() + 1;
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
                            mSignAllowed = false;
                            break;
                        case '.':
                            if (mExponent)
                                unexpected(aSymbol, aParser);
                            mInteger = false;
                            mSignAllowed = false;
                            break;
                        case 'e':
                        case 'E':
                            mInteger = false;
                            mExponent = true;
                            mSignAllowed = true;
                            break;
                        case '-':
                        case '+':
                            if (!mSignAllowed)
                                unexpected(aSymbol, aParser);
                            mSignAllowed = false;
                            break;
                        default:
                            result = StateTransitionPop{};
                            mEnd = aParser.pos();
                            aParser.back();
                            break;
                    }
                    return result;
                }

                // inline value val() const override
                //     {
                //         if (mInteger)
                //             return integer{mParser.data(mBegin, mEnd)};
                //         else
                //             return number{mParser.data(mBegin, mEnd)};
                //     }

                inline value value_move() override
                {
                    if (mInteger)
                        return integer{mParser.data(mBegin, mEnd)};
                    else
                        return number{mParser.data(mBegin, mEnd)};
                }

              private:
                Parser& mParser;
                size_t mBegin, mEnd;
                bool mSignAllowed = true;
                bool mExponent = false;
                bool mInteger = true;

            }; // class NumberHandler

            // ----------------------------------------------------------------------

            class BoolNullHandler : public SymbolHandler
            {
              public:
                inline BoolNullHandler(const char* aExpected, value&& aValue) : mExpected{aExpected}, mValue{std::move(aValue)} {}

                inline HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
                {
                    HandlingResult result = StateTransitionNone{};
                    switch (aSymbol) {
                        case 'r':
                        case 'u':
                        case 'e':
                        case 'a':
                        case 'l':
                        case 's':
                            if (mExpected[mPos] != aSymbol)
                                unexpected(aSymbol, aParser);
                            break;
                        default:
                            if (mExpected[mPos] == 0) {
                                result = StateTransitionPop{};
                                aParser.back();
                            }
                            else {
                                result = SymbolHandler::handle(aSymbol, aParser);
                            }
                    }
                    ++mPos;
                    return result;
                }

                // inline value val() const override { return mValue; }
                inline value value_move() override { return std::move(mValue); }

              private:
                const char* mExpected;
                value mValue;

                size_t mPos = 0;

            }; // class BoolNullHandler

            // ----------------------------------------------------------------------

            class ObjectHandler : public SymbolHandler
            {
              private:
                enum class Expected { Key, KeyAfterComma, Value, Colon, Comma };

              public:
                inline ObjectHandler() {}

                inline HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
                {
                    HandlingResult result = StateTransitionNone{};
                    switch (aSymbol) {
                        case '}':
                            switch (mExpected) {
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
                            switch (mExpected) {
                                case Expected::Comma:
                                    mExpected = Expected::KeyAfterComma;
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
                            if (mExpected == Expected::Colon) {
                                mExpected = Expected::Value;
                                result = std::make_unique<ValueHandler>();
                            }
                            else
                                unexpected(aSymbol, aParser);
                            break;
                        case '"':
                            switch (mExpected) {
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

                // inline value val() const override { return mValue; }
                inline value value_move() override { return std::move(mValue); }

                inline void subvalue(value&& aSubvalue, Parser& aParser) override
                {
                    // std::cerr << "ObjectHandler::subvalue " << aSubvalue.to_json() << '\n';
                    switch (mExpected) {
                        case Expected::Key:
                        case Expected::KeyAfterComma:
                            mKey = std::move(aSubvalue);
                            mExpected = Expected::Colon;
                            break;
                        case Expected::Value:
                            mValue.insert(std::move(mKey), std::move(aSubvalue));
                            mExpected = Expected::Comma;
                            break;
                        case Expected::Comma:
                        case Expected::Colon:
                            internal_error(aParser);
                    }
                }

              private:
                object mValue;
                value mKey;
                Expected mExpected = Expected::Key;

            }; // class ObjectHandler

            // ----------------------------------------------------------------------

            class ArrayHandler : public SymbolHandler
            {
              private:
                enum class Expected { Value, ValueAfterComma, Comma };

              public:
                inline ArrayHandler() {}

                inline HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
                {
                    HandlingResult result = StateTransitionNone{};
                    switch (aSymbol) {
                        case ']':
                            switch (mExpected) {
                                case Expected::Value:
                                case Expected::Comma:
                                    result = StateTransitionPop{};
                                    break;
                                case Expected::ValueAfterComma:
                                    error(aParser, "unexpected " + std::string{aSymbol} + " -- did you forget to remove last comma?");
                            }
                            break;
                        case ',':
                            switch (mExpected) {
                                case Expected::Value:
                                    error(aParser, "unexpected comma right after the beginning of an array");
                                case Expected::Comma:
                                    mExpected = Expected::ValueAfterComma;
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
                            switch (mExpected) {
                                case Expected::Value:
                                case Expected::ValueAfterComma:
                                    result = std::make_unique<ValueHandler>();
                                    aParser.back();
                                    mExpected = Expected::Comma;
                                    break;
                                case Expected::Comma:
                                    error(aParser, "unexpected " + std::string{aSymbol} + " -- did you forget comma?");
                            }
                            break;
                    }
                    return result;
                }

                // inline value val() const override { return mValue; }
                inline value value_move() override { return std::move(mValue); }

                inline void subvalue(value&& aSubvalue, Parser& /*aParser*/) override { mValue.insert(std::move(aSubvalue)); }

              private:
                array mValue;
                Expected mExpected = Expected::Value;

            }; // class ArrayHandler

            // ----------------------------------------------------------------------

            class ToplevelHandler : public ValueHandler
            {
              public:
                inline HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
                {
                    HandlingResult result;
                    if (value_read())
                        result = SymbolHandler::handle(aSymbol, aParser);
                    else
                        result = ValueHandler::handle(aSymbol, aParser);
                    return result;
                }

            }; // class ToplevelHandler

        } // namespace parser_pop
    }     // namespace v1
} // namespace rjson

// ----------------------------------------------------------------------

#ifndef __clang__
namespace std
{
      // gcc 7.2 wants those, if we derive from std::variant
    template<> struct variant_size<rjson::v1::parser_pop::HandlingResult> : variant_size<rjson::v1::parser_pop::HandlingResultBase> {};
    template<size_t _Np> struct variant_alternative<_Np, rjson::v1::parser_pop::HandlingResult> : variant_alternative<_Np, rjson::v1::parser_pop::HandlingResultBase> {};
}
#endif

// ----------------------------------------------------------------------

// inline rjson::v1::value rjson::v1::parser_pop::Parser::result() const
// {
//     return mHandlers.top()->val();

// } // rjson::v1::parser_pop::Parser::result

// ----------------------------------------------------------------------

inline rjson::v1::value rjson::v1::parser_pop::Parser::result_move()
{
    return mHandlers.top()->value_move();

} // rjson::v1::parser_pop::Parser::result

// ----------------------------------------------------------------------

namespace rjson
{
    namespace v1
    {
        namespace parser_pop
        {

            template <typename S> static inline rjson::v1::value parse_string_impl(S aJsonData, rjson::v1::remove_comments aRemoveComments)
            {
                Parser parser{};
                parser.parse(aJsonData);
                parser.remove_emacs_indent();
                if (aRemoveComments == rjson::v1::remove_comments::Yes)
                    parser.remove_comments();
                return parser.result_move();
            }

        } // namespace parser_pop
    }     // namespace v1
} // namespace rjson

rjson::v1::value rjson::v1::parser_pop::parse_string(std::string aJsonData, rjson::v1::remove_comments aRemoveComments)
{
    return parse_string_impl(aJsonData, aRemoveComments);

} // rjson::v1::parser_pop::parse_string

// ----------------------------------------------------------------------

rjson::v1::value rjson::v1::parser_pop::parse_string(const std::string_view& aJsonData, remove_comments aRemoveComments)
{
    return parse_string_impl(aJsonData, aRemoveComments);

} // rjson::v1::parser_pop::parse_string

// ----------------------------------------------------------------------

rjson::v1::value rjson::v1::parser_pop::parse_string(const char* aJsonData, rjson::v1::remove_comments aRemoveComments)
{
    return parse_string_impl(aJsonData, aRemoveComments);

} // rjson::v1::parser_pop::parse_string

// ----------------------------------------------------------------------

rjson::v1::parser_pop::HandlingResult rjson::v1::parser_pop::ValueHandler::handle(std::string_view::value_type aSymbol, Parser& aParser)
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
          case '-': case '+': case '.':
          case '0': case '1': case '2': case '3': case '4': case '5':
          case '6': case '7': case '8': case '9':
              result = std::make_unique<NumberHandler>(aParser);
              aParser.back();
              break;
          case 't':
              result = std::make_unique<BoolNullHandler>("rue", rjson::v1::boolean{true});
              break;
          case 'f':
              result = std::make_unique<BoolNullHandler>("alse", rjson::v1::boolean{false});
              break;
          case 'n':
              result = std::make_unique<BoolNullHandler>("ull", rjson::v1::null{});
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

} // rjson::v1::parser_pop::ValueHandler::handle

// ----------------------------------------------------------------------

rjson::v1::parser_pop::Parser::Parser()
    : mPos{0}, mLine{1}, mColumn{1}
{
    mHandlers.emplace(new ToplevelHandler);
}

// ----------------------------------------------------------------------

inline void rjson::v1::parser_pop::Parser::pop()
{
    auto value{mHandlers.top()->value_move()};
    mHandlers.pop();
    mHandlers.top()->subvalue(std::move(value), *this);

} // rjson::v1::parser_pop::Parser::pop

// ----------------------------------------------------------------------

void rjson::v1::parser_pop::Parser::parse(std::string_view aJsonData)
{
    mSource = aJsonData;
    for (mPos = 0; mPos < mSource.size(); ++mPos, ++mColumn) {
        auto symbol = mSource[mPos];
          // std::cerr << "HANDLE " << symbol << ' ' << mLine << ':' << mColumn << '\n';
        auto handling_result = mHandlers.top()->handle(symbol, *this);
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::unique_ptr<SymbolHandler>>) {
                    this->mHandlers.push(std::forward<T>(arg));
                }
                else if constexpr (std::is_same_v<T, StateTransitionPop>) {
                    pop();
                }
            }, handling_result);
        }
        if (mHandlers.size() > 1)
            pop();

} // rjson::v1::parser_pop::Parser::parse

// ----------------------------------------------------------------------

void rjson::v1::parser_pop::Parser::remove_emacs_indent()
{
    auto& value = dynamic_cast<ToplevelHandler*>(mHandlers.top().get())->val();
    if (auto* top_obj = std::get_if<rjson::v1::object>(&value); top_obj) {
        try {
            top_obj->delete_field("_");
        }
        catch (rjson::v1::field_not_found&) {
        }
    }

} // rjson::v1::parser_pop::Parser::remove_emacs_indent

// ----------------------------------------------------------------------

inline void rjson::v1::parser_pop::Parser::remove_comments()
{
    dynamic_cast<ToplevelHandler*>(mHandlers.top().get())->val().remove_comments();

} // rjson::v1::parser_pop::Parser::remove_comments

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
