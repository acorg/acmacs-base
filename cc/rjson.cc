#include <iostream>
#include <stack>
#include <memory>

#include "string.hh"
#include "rjson.hh"

// ----------------------------------------------------------------------

#ifdef __clang__
const char* std::bad_variant_access::what() const noexcept { return "bad_variant_access"; }
#endif

// ----------------------------------------------------------------------

namespace rjson::implementation
{
class SymbolHandler;

class StateTransitionNone {};
class StateTransitionPop {};

class HandlingResult : public std::variant<StateTransitionNone, StateTransitionPop, std::unique_ptr<SymbolHandler>>
{
 public:
    using std::variant<StateTransitionNone, StateTransitionPop, std::unique_ptr<SymbolHandler>>::variant;

}; // class HandlingResult

// ----------------------------------------------------------------------

class Parser
{
 public:
    Parser(std::string_view&& aSource);

    void parse();

    rjson::value result() const;
    inline size_t pos() const { return mPos; }
    inline size_t line() const { return mLine; }
    inline size_t column() const { return mColumn; }
    inline std::string_view::value_type previous() const { if (mPos) return mSource[mPos - 1]; throw rjson::Error(line(), pos(), "internal: no previous at the beginning of parsing"); }
    inline std::string_view view(size_t aBegin, size_t aEnd) const { return {mSource.data() + aBegin, aEnd - aBegin}; }
    inline void back() { if (!mPos) throw rjson::Error(line(), pos(), "internal: cannot back at the beginning of parsing"); --mPos; }
    inline void newline() { ++mLine; mColumn = 0; }

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
    virtual ~SymbolHandler() {}

    virtual HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser)
        {
            switch (aSymbol) {
              case ' ':
              case '\t':
              case '\r':
                  break;
              case '\n':
                  aParser.newline();
                  break;
              default:
                  throw rjson::Error(aParser.line(), aParser.column(), "unexpected symbol: " + std::string(1, aSymbol) + " (" + ::string::to_hex_string(static_cast<unsigned char>(aSymbol), ::string::ShowBase, ::string::Uppercase) + ")");
            }
            return StateTransitionNone{};
        }

    virtual rjson::value value() const { const rjson::value v{rjson::null{}}; return v; }
    virtual void subvalue(rjson::value&& /*aSubvalue*/) {}
    // virtual void complete(Parser& /*aParser*/) {}

}; // class SymbolHandler

// ----------------------------------------------------------------------

class StringEscapeHandler : public SymbolHandler
{
 public:
    HandlingResult handle(std::string_view::value_type /*aSymbol*/, Parser& /*aParser*/) override
        {
            return StateTransitionPop{};
        }

}; // class StringEscapeHandler

// ----------------------------------------------------------------------

class StringHandler : public SymbolHandler
{
 public:
    StringHandler(Parser& aParser) : mParser{aParser}, mBegin{aParser.pos() + 1}, mEnd{0} {}

    HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
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
                  result = SymbolHandler::handle(aSymbol, aParser);
                  break;
              default:
                  break;
            }
            return result;
        }

    rjson::value value() const override
        {
            const rjson::value v{rjson::string{mParser.view(mBegin, mEnd)}};
            return v;
        }

 private:
    Parser& mParser;
    size_t mBegin, mEnd;

}; // class StringHandler

// ----------------------------------------------------------------------

class NumberExponentHandler : public SymbolHandler
{
}; // class NumberExponentHandler

class NumberHandler : public SymbolHandler
{
 public:
    NumberHandler(Parser& aParser) : mParser{aParser}, mBegin{aParser.pos()}, mEnd{0} {}

    HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
        {
            HandlingResult result = StateTransitionNone{};
            mEnd = aParser.pos() + 1;
            switch (aSymbol) {
              case '0': case '1': case '2': case '3': case '4': case '5':
              case '6': case '7': case '8': case '9':
                  break;
              case '.':
                  mInteger = false;
                  break;
              case 'e':
              case 'E':
                  mInteger = false;
                  result = std::make_unique<NumberExponentHandler>();
                  break;
              case '-':
                  if (aParser.pos() != mBegin)
                      result = SymbolHandler::handle(aSymbol, aParser);
                  break;
              default:
                  result = StateTransitionPop{};
                  mEnd = aParser.pos();
                  aParser.back();
                  break;
            }
            return result;
        }

    rjson::value value() const override
        {
            rjson::value v;
            if (mInteger)
                v = rjson::integer{mParser.view(mBegin, mEnd)};
            else
                v = rjson::number{mParser.view(mBegin, mEnd)};
            return v;
        }

    // void complete(Parser& aParser) override
    //     {
    //         if (mEnd == 0)
    //             mEnd = aParser.pos();
    //     }

 private:
    Parser& mParser;
    size_t mBegin, mEnd;
    bool mInteger = true;

}; // class NumberHandler

// ----------------------------------------------------------------------

class ToplevelHandler : public SymbolHandler
{
 public:
    HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
        {
            HandlingResult result = StateTransitionNone{};
            if (mValueRead) {
                result = SymbolHandler::handle(aSymbol, aParser);
            }
            else {
                switch (aSymbol) {
                  case '"':
                      result = std::make_unique<StringHandler>(aParser);
                      break;
                  case '-':
                  case '0': case '1': case '2': case '3': case '4': case '5':
                  case '6': case '7': case '8': case '9':
                      result = std::make_unique<NumberHandler>(aParser);
                      aParser.back();
                      break;
                  case 't':
                      result = SymbolHandler::handle(aSymbol, aParser);
                      break;
                  case 'f':
                      result = SymbolHandler::handle(aSymbol, aParser);
                      break;
                  case 'n':
                      result = SymbolHandler::handle(aSymbol, aParser);
                      break;
                  default:
                      result = SymbolHandler::handle(aSymbol, aParser);
                      break;
                }
            }
            return result;
        }

    void subvalue(rjson::value&& aSubvalue) override
        {
            mValue = aSubvalue;
            mValueRead = true;
        }

    rjson::value value() const override { return mValue; }

 private:
    bool mValueRead = false;
    rjson::value mValue;

}; // class ToplevelHandler

// ----------------------------------------------------------------------

Parser::Parser(std::string_view&& aSource)
    : mSource{std::move(aSource)}, mPos{0}, mLine{1}, mColumn{1}
{
    mHandlers.emplace(new ToplevelHandler);

} // Parser::Parser

// ----------------------------------------------------------------------

void Parser::pop()
{
    // mHandlers.top()->complete(*this);
    auto value = mHandlers.top()->value();
    mHandlers.pop();
    mHandlers.top()->subvalue(std::move(value));

} // Parser::pop

// ----------------------------------------------------------------------

void Parser::parse()
{
    for (mPos = 0; mPos < mSource.size(); ++mPos, ++mColumn) {
        auto symbol = mSource[mPos];
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

} // Parser::parse

// ----------------------------------------------------------------------

inline rjson::value Parser::result() const
{
    return mHandlers.top()->value();

} // Parser::result

}

// ----------------------------------------------------------------------

rjson::number::number(std::string_view&& aData)
    : mValue{std::stod(static_cast<std::string>(aData))}
{
} // rjson::number::number

// ----------------------------------------------------------------------

rjson::integer::integer(std::string_view&& aData)
    : mValue{std::stol(static_cast<std::string>(aData))}
{
} // rjson::integer::integer

// ----------------------------------------------------------------------

rjson::value rjson::parse(std::string aData)
{
    implementation::Parser parser{aData};
    parser.parse();
    return parser.result();

} // rjson::parse

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
