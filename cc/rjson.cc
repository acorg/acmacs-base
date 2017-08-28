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
    inline std::string_view view(size_t aBegin, size_t aEnd) const { return {mSource.data() + aBegin, aEnd - aBegin}; }
    inline void newline() { ++mLine; mColumn = 0; }

 private:
    std::string_view mSource;
    size_t mPos, mLine, mColumn;
    std::stack<std::unique_ptr<SymbolHandler>> mHandlers;

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
                  throw rjson::Error(aParser.line(), aParser.column(), "unexpected symbol: " + std::string(1, aSymbol) + " (" + string::to_hex_string(static_cast<unsigned char>(aSymbol), string::ShowBase, string::Uppercase) + ")");
            }
            return StateTransitionNone{};
        }

    virtual rjson::value value() const { const rjson::value v{rjson::null{}}; return v; }
    virtual void subvalue(rjson::value&& /*aSubvalue*/) {}

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
    StringHandler(Parser& aParser)
        : mParser{aParser}, mBegin{aParser.pos() + 1}, mEnd{0}
        {
        }

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

void Parser::parse()
{
    for (auto symbol: mSource) {
        auto handling_result = mHandlers.top()->handle(symbol, *this);
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::unique_ptr<SymbolHandler>>) {
                this->mHandlers.push(std::forward<T>(arg));
            }
            else if constexpr (std::is_same_v<T, StateTransitionPop>) {
                auto value = mHandlers.top()->value();
                this->mHandlers.pop();
                mHandlers.top()->subvalue(std::move(value));
                }
        }, handling_result);
        ++mPos;
        ++mColumn;
    }

} // Parser::parse

// ----------------------------------------------------------------------

inline rjson::value Parser::result() const
{
    return mHandlers.top()->value();

} // Parser::result

// ----------------------------------------------------------------------

rjson::value rjson::parse(std::string aData)
{
    Parser parser{aData};
    parser.parse();
    return parser.result();

} // rjson::parse

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
