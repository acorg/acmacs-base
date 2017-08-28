#pragma once

#include <stdexcept>
#include <variant>
#include <string>
#include <string_view>

// ----------------------------------------------------------------------

namespace rjson
{
    class value;

    class object
    {
     public:
        inline std::string to_string() const { return "{}"; }
    };

    class array
    {
     public:
        inline std::string to_string() const { return "[]"; }
    };

    class string
    {
     public:
        inline string(std::string_view&& aData) : mData{std::move(aData)} {}
        inline std::string to_string() const { using namespace std::literals; return "\""s + static_cast<std::string>(mData) + "\""; }

     private:
        std::string_view mData;
    };

    class number
    {
     public:
        inline std::string to_string() const { return "<number>"; }
    };

    class integer : public number
    {
     public:
        inline std::string to_string() const { return "<integer>"; }
    };

    class boolean
    {
     public:
        inline boolean(bool aValue) : mValue{aValue} {}
        inline std::string to_string() const { return mValue ? "true" : "false"; }

     private:
        bool mValue;
    };

    class null
    {
     public:
        inline null() {}
        inline std::string to_string() const { return "null"; }
    };

    class value : public std::variant<null, object, array, string, integer, number, boolean> // null must be the first alternative, it is the default value
    {
     public:
        std::string to_string() const
            {
                return std::visit([](auto&& arg) -> std::string { return arg.to_string(); }, *this);
            }
    };

    class Error : public std::exception
    {
     public:
        inline Error(size_t aLine, size_t aColumn, std::string aMessage)
            : mMessage{std::to_string(aLine) + ":" + std::to_string(aColumn) + ": " + aMessage} //, mLine{aLine}, mColumn{aColumn}
            {}

        const char* what() const noexcept override { return mMessage.c_str(); }

     private:
        std::string mMessage;
          //size_t mLine, mColumn;

    }; // class Error

    value parse(std::string aData);

} // namespace rjson

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
