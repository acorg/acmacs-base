#pragma once

#include <stdexcept>
#include <variant>
#include <string>
#include <string_view>
#include <vector>

#include "float.hh"

// ----------------------------------------------------------------------

namespace rjson
{
    namespace implementation { class NumberHandler; }

    class value;
    class string;

    class object
    {
     public:
        std::string to_string() const;

        void insert(value&& aKey, value&& aValue);

     private:
        std::vector<std::pair<string, value>> mContent;
    };

    class array
    {
     public:
        std::string to_string() const;

        void insert(value&& aValue);

     private:
        std::vector<value> mContent;
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
        inline std::string to_string() const { return double_to_string(mValue); }
        inline operator double() const { return mValue; }

     private:
        inline number(std::string_view&& aData) : mValue{std::stod(static_cast<std::string>(aData))} {}

        double mValue;

        friend class implementation::NumberHandler;
    };

    class integer
    {
     public:
        inline std::string to_string() const { return std::to_string(mValue); }
        inline operator double() const { return mValue; }
        inline operator long() const { return mValue; }

     private:
        inline integer(std::string_view&& aData) : mValue{std::stol(static_cast<std::string>(aData))} {}

        long mValue;

        friend class implementation::NumberHandler;
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

    using value_base = std::variant<null, object, array, string, integer, number, boolean>; // null must be the first alternative, it is the default value;

    class value : public value_base
    {
     public:
        using value_base::operator=;
        using value_base::value_base;
        inline value(const value&) = default; // gcc7 wants this, otherwise it is deleted
        inline value& operator=(const value&) = default; // gcc7 wants this, otherwise it is deleted

        std::string to_string() const;
    };

    class Error : public std::exception
    {
     public:
        // inline Error(size_t aLine, size_t aColumn, std::string aMessage)
        //     : mMessage{std::to_string(aLine) + ":" + std::to_string(aColumn) + ": " + aMessage} //, mLine{aLine}, mColumn{aColumn}
        //     {}

        inline Error(size_t aLine, size_t aColumn, std::string&& aMessage)
            : mMessage{std::to_string(aLine) + ":" + std::to_string(aColumn) + ": " + std::move(aMessage)} //, mLine{aLine}, mColumn{aColumn}
            {}

        inline const char* what() const noexcept override { return mMessage.c_str(); }

     private:
        std::string mMessage;
          //size_t mLine, mColumn;

    }; // class Error

    value parse(std::string aData);

      // ----------------------------------------------------------------------

    inline void object::insert(value&& aKey, value&& aValue)
    {
        mContent.emplace_back(std::get<rjson::string>(aKey), aValue);
    }

    inline void array::insert(value&& aValue)
    {
        mContent.push_back(std::move(aValue));
    }

} // namespace rjson

// ----------------------------------------------------------------------

#if __GNUC__ == 7
namespace std
{
      // gcc 7.2 wants those, if we derive from std::variant
    template<> struct variant_size<rjson::value> : variant_size<rjson::value_base> {};
    template<size_t _Np> struct variant_alternative<_Np, rjson::value> : variant_alternative<_Np, rjson::value_base> {};
}
#endif

// ----------------------------------------------------------------------

inline std::string rjson::value::to_string() const
{
    return std::visit([](auto&& arg) -> std::string { return arg.to_string(); }, *this);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
