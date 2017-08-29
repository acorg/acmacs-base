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

    inline void back()
        {
            if (!mPos)
                throw rjson::Error(line(), pos(), "internal: cannot back at the beginning of parsing");
            --mPos;
            --mColumn;
            if (mColumn == 0)
                --mLine;
            // std::cerr << "BACK " << mLine << ':' << mColumn << ' ' << mPos << '\n';
        }

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

    [[noreturn]] inline void unexpected(std::string_view::value_type aSymbol, Parser& aParser) const
        {
            throw rjson::Error(aParser.line(), aParser.column(), "unexpected symbol: " + std::string(1, aSymbol) + " (" + ::string::to_hex_string(static_cast<unsigned char>(aSymbol), ::string::ShowBase, ::string::Uppercase) + ")");
        }

    [[noreturn]] inline void error(Parser& aParser, std::string&& aMessage) const
        {
            throw rjson::Error(aParser.line(), aParser.column(), std::move(aMessage));
        }

    [[noreturn]] inline void internal_error(Parser& aParser) const
        {
            throw rjson::Error(aParser.line(), aParser.column(), "internal error");
        }

    inline void newline(Parser& aParser) const
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

    virtual rjson::value value() const { return rjson::null{}; }
    virtual void subvalue(rjson::value&& /*aSubvalue*/, Parser& /*aParser*/) {}
    // virtual void complete(Parser& /*aParser*/) {}

}; // class SymbolHandler

// ----------------------------------------------------------------------

class ValueHandler : public SymbolHandler
{
 public:
    HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override;

    void subvalue(rjson::value&& aSubvalue, Parser& /*aParser*/) override
        {
            mValue = aSubvalue;
            mValueRead = true;
        }

    rjson::value value() const override { return mValue; }

 protected:
    bool value_read() const { return mValueRead; }

 private:
    bool mValueRead = false;
    rjson::value mValue;

}; // class ValueHandler

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
                  newline(aParser);
                  break;
              default:
                  break;
            }
            return result;
        }

    rjson::value value() const override
        {
            return rjson::string{mParser.view(mBegin, mEnd)};
        }

 private:
    Parser& mParser;
    size_t mBegin, mEnd;

}; // class StringHandler

// ----------------------------------------------------------------------

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

    rjson::value value() const override
        {
            if (mInteger)
                return rjson::integer{mParser.view(mBegin, mEnd)};
            else
                return rjson::number{mParser.view(mBegin, mEnd)};
        }

    // void complete(Parser& aParser) override
    //     {
    //         if (mEnd == 0)
    //             mEnd = aParser.pos();
    //     }

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
    inline BoolNullHandler(const char* aExpected, rjson::value&& aValue) : mExpected{aExpected}, mValue{std::move(aValue)} {}

    HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
        {
            HandlingResult result = StateTransitionNone{};
            switch (aSymbol) {
              case 'r': case 'u': case 'e':
              case 'a': case 'l': case 's':
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

    rjson::value value() const override { return mValue; }

 private:
    const char* mExpected;
    rjson::value mValue;

    size_t mPos = 0;

}; // class BoolNullHandler

// ----------------------------------------------------------------------

class ObjectHandler : public SymbolHandler
{
 private:
    enum class Expected { Key, KeyAfterComma, Value, Colon, Comma };

 public:
    inline ObjectHandler() {}

    HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
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
                        error(aParser, "unexpected " + std::string(1, aSymbol) + " -- did you forget to remove last comma?");
                    case Expected::Value:
                    case Expected::Colon:
                        unexpected(aSymbol, aParser);
                  }
                  break;
              case ',':
                  if (mExpected == Expected::Comma)
                      mExpected = Expected::KeyAfterComma;
                  else
                      unexpected(aSymbol, aParser);
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
                        error(aParser, "unexpected " + std::string(1, aSymbol) + " -- did you forget comma?");
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

    rjson::value value() const override { return mValue; }

    void subvalue(rjson::value&& aSubvalue, Parser& aParser) override
        {
            // std::cerr << "ObjectHandler::subvalue " << aSubvalue.to_string() << '\n';
            switch (mExpected) {
              case Expected::Key:
              case Expected::KeyAfterComma:
                  mKey = aSubvalue;
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
    rjson::object mValue;
    rjson::value mKey;
    Expected mExpected = Expected::Key;

}; //class ObjectHandler

// ----------------------------------------------------------------------

class ArrayHandler : public SymbolHandler
{
 public:
    inline ArrayHandler() {}

    HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
        {
            HandlingResult result = StateTransitionNone{};
            return result;
        }

    rjson::value value() const override { return mValue; }

 private:
    rjson::array mValue;

}; //class ArrayHandler

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

}; //class ToplevelHandler

// ----------------------------------------------------------------------

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
          case '-': case '+': case '.':
          case '0': case '1': case '2': case '3': case '4': case '5':
          case '6': case '7': case '8': case '9':
              result = std::make_unique<NumberHandler>(aParser);
              aParser.back();
              break;
          case 't':
              result = std::make_unique<BoolNullHandler>("rue", rjson::boolean{true});
              break;
          case 'f':
              result = std::make_unique<BoolNullHandler>("alse", rjson::boolean{false});
              break;
          case 'n':
              result = std::make_unique<BoolNullHandler>("ull", rjson::null{});
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

} // ValueHandler::handle

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
    mHandlers.top()->subvalue(std::move(value), *this);

} // Parser::pop

// ----------------------------------------------------------------------

void Parser::parse()
{
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

} // Parser::parse

// ----------------------------------------------------------------------

inline rjson::value Parser::result() const
{
    return mHandlers.top()->value();

} // Parser::result

}

// ----------------------------------------------------------------------

rjson::value rjson::parse(std::string aData)
{
      // std::cerr << "PARSE %" << aData << "%\n";
    implementation::Parser parser{aData};
    parser.parse();
    return parser.result();

} // rjson::parse

// ----------------------------------------------------------------------

std::string rjson::object::to_string() const
{
    std::string result(1, '{');
    for (auto [key, val]: mContent) {
        result.append(key.to_string());
        result.append(1, ':');
        result.append(val.to_string());
        result.append(1, ',');
    }
    if (result.back() == ',')
        result.back() = '}';
    else
        result.append(1, '}');
    return result;

} // rjson::object::to_string

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
