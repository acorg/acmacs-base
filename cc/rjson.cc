#include <iostream>
#include <stack>
#include <memory>

#include "string.hh"
#include "read-file.hh"
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

        void parse(std::string aJsonData);

        rjson::value result() const;
        void remove_emacs_indent();
        inline size_t pos() const { return mPos; }
        inline size_t line() const { return mLine; }
        inline size_t column() const { return mColumn; }
        inline std::string_view::value_type previous() const { if (mPos) return mSource[mPos - 1]; throw rjson::parse_error(line(), pos(), "internal: no previous at the beginning of parsing"); }
        inline std::string data(size_t aBegin, size_t aEnd) const { return {mSource.data() + aBegin, aEnd - aBegin}; }

        inline void back()
            {
                if (!mPos)
                    throw rjson::parse_error(line(), pos(), "internal: cannot back at the beginning of parsing");
                --mPos;
                --mColumn;
                if (mColumn == 0)
                    --mLine;
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
                throw rjson::parse_error(aParser.line(), aParser.column(), "unexpected symbol: " + std::string{aSymbol} + " (" + ::string::to_hex_string(static_cast<unsigned char>(aSymbol), ::string::ShowBase, ::string::Uppercase) + ")");
            }

        [[noreturn]] inline void error(Parser& aParser, std::string&& aMessage) const
            {
                throw rjson::parse_error(aParser.line(), aParser.column(), std::move(aMessage));
            }

        [[noreturn]] inline void internal_error(Parser& aParser) const
            {
                throw rjson::parse_error(aParser.line(), aParser.column(), "internal error");
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
        rjson::value& value() { return mValue; }

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
                return rjson::string{mParser.data(mBegin, mEnd)};
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
                    return rjson::integer{mParser.data(mBegin, mEnd)};
                else
                    return rjson::number{mParser.data(mBegin, mEnd)};
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

        rjson::value value() const override { return mValue; }

        void subvalue(rjson::value&& aSubvalue, Parser& aParser) override
            {
                  // std::cerr << "ObjectHandler::subvalue " << aSubvalue.to_json() << '\n';
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
     private:
        enum class Expected { Value, ValueAfterComma, Comma };

     public:
        inline ArrayHandler() {}

        HandlingResult handle(std::string_view::value_type aSymbol, Parser& aParser) override
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
                  case ' ': case '\t':
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

        rjson::value value() const override { return mValue; }

        void subvalue(rjson::value&& aSubvalue, Parser& /*aParser*/) override
            {
                mValue.insert(std::move(aSubvalue));
            }

     private:
        rjson::array mValue;
        Expected mExpected = Expected::Value;

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

} // namespace rjson::implementation

// ----------------------------------------------------------------------

#if __GNUC__ == 7
namespace std
{
      // gcc 7.2 wants those, if we derive from std::variant
    template<> struct variant_size<rjson::implementation::HandlingResult> : variant_size<rjson::implementation::HandlingResultBase> {};
    template<size_t _Np> struct variant_alternative<_Np, rjson::implementation::HandlingResult> : variant_alternative<_Np, rjson::implementation::HandlingResultBase> {};
}
#endif

// ----------------------------------------------------------------------

rjson::implementation::HandlingResult rjson::implementation::ValueHandler::handle(std::string_view::value_type aSymbol, Parser& aParser)
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

} // rjson::implementation::ValueHandler::handle

// ----------------------------------------------------------------------

rjson::implementation::Parser::Parser()
    : mPos{0}, mLine{1}, mColumn{1}
{
    mHandlers.emplace(new ToplevelHandler);
}

// ----------------------------------------------------------------------

void rjson::implementation::Parser::pop()
{
    auto value = mHandlers.top()->value();
    mHandlers.pop();
    mHandlers.top()->subvalue(std::move(value), *this);

} // rjson::implementation::Parser::pop

// ----------------------------------------------------------------------

void rjson::implementation::Parser::parse(std::string aJsonData)
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

} // rjson::implementation::Parser::parse

// ----------------------------------------------------------------------

inline rjson::value rjson::implementation::Parser::result() const
{
    return mHandlers.top()->value();

} // rjson::implementation::Parser::result

// ----------------------------------------------------------------------

void rjson::implementation::Parser::remove_emacs_indent()
{
    auto& value = dynamic_cast<ToplevelHandler*>(mHandlers.top().get())->value();
    if (auto* top_obj = std::get_if<rjson::object>(&value); top_obj) {
        try {
            top_obj->delete_field("_");
        }
        catch (rjson::object::field_not_found&) {
        }
    }

} // rjson::implementation::Parser::remove_emacs_indent

// ----------------------------------------------------------------------

rjson::value rjson::parse_string(std::string aJsonData)
{
    implementation::Parser parser{};
    parser.parse(aJsonData);
    parser.remove_emacs_indent();
    return parser.result();

} // rjson::parse_string

// ----------------------------------------------------------------------

rjson::value rjson::parse_file(std::string aFilename)
{
    return parse_string(acmacs_base::read_file(aFilename, true));

} // rjson::parse_file

// ----------------------------------------------------------------------

static inline bool is_simple(const rjson::value& aValue, bool dive = true)
{
    auto visitor = [dive](auto&& arg) -> bool {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, rjson::object>) {
            return !arg.get_ref_not_set(rjson::object::force_pp_key, rjson::boolean{false}) && (arg.empty() || (dive && std::all_of(arg.begin(), arg.end(), [](const auto& kv) -> bool { return is_simple(kv.second, false); })));
        }
        else if constexpr (std::is_same_v<T, rjson::array>) {
            return arg.empty() || (dive && std::all_of(arg.begin(), arg.end(), [](const auto& v) -> bool { return is_simple(v, false); }));
        }
        else {
            return true;
        }
    };
    return std::visit(visitor, aValue);
}

// ----------------------------------------------------------------------

std::string rjson::object::to_json(bool space_after_comma) const
{
    std::string result(1, '{');
    size_t size_at_comma = 0;
    for (const auto& [key, val]: mContent) {
        result.append(key.to_json());
        result.append(1, ':');
        if (space_after_comma)
            result.append(1, ' ');
        result.append(val.to_json());
        size_at_comma = result.size() + 1;
        result.append(1, ',');
        if (space_after_comma)
            result.append(1, ' ');
    }
    if (result.back() == ',' || result.back() == ' ') {
        result.resize(size_at_comma);
        result.back() = '}';
    }
    else
        result.append(1, '}');
    return result;

} // rjson::object::to_json

// ----------------------------------------------------------------------

std::string rjson::object::to_json_pp(size_t indent, json_pp_emacs_indent emacs_indent, size_t prefix) const
{
    if (is_simple(*this))
        return to_json(true);

    std::string result;
    if (emacs_indent == json_pp_emacs_indent::yes && indent) {
        result.append(1, '{');
        result.append(indent - 1, ' ');
        result.append("\"_\": \"-*- js-indent-level: " + std::to_string(indent) + " -*-\",\n");
    }
    else {
        result.append("{\n");
    }
    result.append(prefix + indent, ' ');
    size_t size_before_comma = result.size();
    for (const auto& [key, val]: mContent) {
        if (key != force_pp_key) {
            result.append(key.to_json());
            result.append(": ");
            result.append(val.to_json_pp(indent, json_pp_emacs_indent::no, prefix + indent));
            size_before_comma = result.size();
            result.append(",\n");
            result.append(prefix + indent, ' ');
        }
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

} // rjson::object::to_json_pp

// ----------------------------------------------------------------------

std::string rjson::array::to_json(bool space_after_comma) const
{
    std::string result(1, '[');
    for (const auto& val: mContent) {
        result.append(val.to_json());
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

} // rjson::array::to_json

// ----------------------------------------------------------------------

std::string rjson::array::to_json_pp(size_t indent, json_pp_emacs_indent, size_t prefix) const
{
    if (is_simple(*this))
        return to_json(true);

    std::string result("[\n");
    result.append(prefix + indent, ' ');
    size_t size_before_comma = 0;
    for (const auto& val: mContent) {
        result.append(val.to_json_pp(indent, json_pp_emacs_indent::no, prefix + indent));
        size_before_comma = result.size();
        result.append(",\n");
        result.append(prefix + indent, ' ');
    }
    if (result.back() == ' ') {
        result.resize(size_before_comma);
        result.append(1, '\n');
        result.append(prefix, ' ');
        result.append(1, ']');
    }
    else
        result.append(1, ']');
    return result;

} // rjson::array::to_json_pp

// ----------------------------------------------------------------------

rjson::value& rjson::value::update(const rjson::value& to_merge)
{
    auto visitor = [this,&to_merge](auto&& arg1, auto&& arg2) {
        using T1 = std::decay_t<decltype(arg1)>;
        using T2 = std::decay_t<decltype(arg2)>;
        if constexpr (std::is_same_v<T1, T2>) {
            arg1.update(arg2);
        }
        else {
            throw merge_error(std::string{"cannot merge two rjson values of different types: %"} + this->to_json() + "% and %" + to_merge.to_json());
        }
    };

    std::visit(visitor, *this, to_merge);
    return *this;

} // rjson::value::update

// ----------------------------------------------------------------------

void rjson::object::update(const rjson::object& to_merge)
{
    for (const auto& [new_key, new_value]: to_merge) {
        try {
            value& old_value = get_ref(new_key);
            if (auto [old_n, old_i, new_n, new_i] = std::make_tuple(std::get_if<number>(&old_value), std::get_if<integer>(&old_value), std::get_if<number>(&new_value), std::get_if<integer>(&new_value));
                (old_n || old_i) && (new_n || new_i)) {
                  // number replaced with integer and vice versa
                set_field(new_key, new_value);
            }
            else {
                old_value.update(new_value);
            }
        }
        catch (field_not_found&) {
            set_field(new_key, new_value);
        }
    }

} // rjson::object::update

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
