#pragma once

#include <stdexcept>
#include <variant>
#include <string>

// ----------------------------------------------------------------------

namespace rjson
{
    class value;

    class object
    {
    };

    class array
    {
    };

    class string
    {
    };

    class number
    {
    };

    class integer : public number
    {
    };

    class boolean
    {
    };

    class null
    {
    };

    class value : public std::variant<object, array, string, integer, number, boolean, null>
    {
     public:
        std::string to_string() const;
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
