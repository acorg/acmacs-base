#include <iostream>
#include <string_view>
#include <stack>
#include <memory>

#include "string.hh"
#include "rjson.hh"

// ----------------------------------------------------------------------

std::string rjson::value::to_string() const
{
    return "?";

} // rjson::value::to_string

// ----------------------------------------------------------------------

class SymbolHandler;

class StateTransitionNone {};
class StateTransitionPop {};

class HandlingResult : public std::variant<StateTransitionNone, StateTransitionPop, std::unique_ptr<SymbolHandler>>
{
 public:
    using std::variant<StateTransitionNone, StateTransitionPop, std::unique_ptr<SymbolHandler>>::variant;

}; // class HandlingResult

class Parser
{
 public:
    Parser(std::string_view&& aSource);

    void parse();

    inline const rjson::value& result() const { return mResult; }
    inline size_t line() const { return mLine; }
    inline size_t column() const { return mColumn; }
    inline void newline() { ++mLine; mColumn = 0; }

 private:
    std::string_view mSource;
    rjson::value mResult;
    size_t mPos, mLine, mColumn;
    std::stack<std::unique_ptr<SymbolHandler>> mHandlers;

      // friend class SymbolHandler;

}; // class Parser

// ----------------------------------------------------------------------

class SymbolHandler
{
 public:
    virtual ~SymbolHandler() {}

    virtual HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser)
        {
            switch (aSymbol) {
              default:
                  throw rjson::Error(aParser.line(), aParser.column(), "unexpected symbol: " + std::string(1, aSymbol) + " (" + string::to_hex_string(static_cast<unsigned char>(aSymbol), string::ShowBase, string::Uppercase) + ")");
              case ' ':
              case '\t':
              case '\r':
                  break;
              case '\n':
                  std::cerr << string::to_hex_string(static_cast<unsigned char>(aSymbol), string::ShowBase) <<  ' ' << (aSymbol == '\n') << '\n';
                  aParser.newline();
                  break;
            }
            return StateTransitionNone{};
        }
};

class ToplevelHandler : public SymbolHandler
{
};

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
        ++mPos;
        ++mColumn;
    }

} // Parser::parse

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
