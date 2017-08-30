#pragma once

#include <stdexcept>
#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>

#include "float.hh"

// ----------------------------------------------------------------------

namespace rjson
{
    namespace implementation { class NumberHandler; }

    class value;

    class string
    {
     public:
        inline string(std::string_view&& aData) : mData{std::move(aData)} {}
        inline string(std::string aData) : mBuffer{aData}, mData(mBuffer.data(), mBuffer.size()) {}
        inline std::string to_json() const { using namespace std::literals; return "\""s + static_cast<std::string>(mData) + "\""; }
        inline operator std::string() const { return mBuffer; }
        inline string& operator=(std::string aSrc) { mBuffer = aSrc; mData = mBuffer; /* std::string_view(mBuffer.data(), mBuffer.size()); */ return *this; }
        inline bool operator==(const std::string aToCompare) const { return mData == aToCompare; }
        inline bool operator==(const string& aToCompare) const { return mData == aToCompare.mData; }

     private:
        std::string mBuffer;    // in case there is no external buffer
        std::string_view mData;
    };

    class boolean
    {
     public:
        inline boolean(bool aValue) : mValue{aValue} {}
        inline std::string to_json() const { return mValue ? "true" : "false"; }
        inline operator bool() const { return mValue; }

     private:
        bool mValue;
    };

    class null
    {
     public:
        inline null() {}
        inline std::string to_json() const { return "null"; }
    };

    class number
    {
     public:
        inline std::string to_json() const { return double_to_string(mValue); }
        inline operator double() const { return mValue; }

     private:
        inline number(std::string_view&& aData) : mValue{std::stod(static_cast<std::string>(aData))} {}

        double mValue;

        friend class implementation::NumberHandler;
    };

    class integer
    {
     public:
        inline std::string to_json() const { return std::to_string(mValue); }
        inline operator double() const { return mValue; }
        inline operator long() const { return mValue; }

     private:
        inline integer(std::string_view&& aData) : mValue{std::stol(static_cast<std::string>(aData))} {}

        long mValue;

        friend class implementation::NumberHandler;
    };

    class object
    {
     public:
        std::string to_json() const;

        void insert(value&& aKey, value&& aValue);

          // returns reference to the value at the passed key.
          // if key not found, inserts aDefault with the passed key and returns reference to the inserted
        value& get_ref(std::string aKey, value&& aDefault);

     private:
        std::vector<std::pair<string, value>> mContent;
    };

    class array
    {
     public:
        std::string to_json() const;

        void insert(value&& aValue);

     private:
        std::vector<value> mContent;
    };

    using value_base = std::variant<null, object, array, string, integer, number, boolean>; // null must be the first alternative, it is the default value;

    class value : public value_base
    {
     public:
        using value_base::operator=;
        using value_base::value_base;
        inline value(const value&) = default; // gcc7 wants this, otherwise it is deleted
        inline value& operator=(const value&) = default; // gcc7 wants this, otherwise it is deleted

          // returns reference to the value at the passed key.
          // if key not found, inserts aDefault with the passed key and returns reference to the inserted
          // if this is not an object, throws  std::bad_variant_access
        inline value& get_ref(std::string aKey, value&& aDefault)
            {
                return std::get<object>(*this).get_ref(aKey, std::forward<value>(aDefault));
            }

        std::string to_json() const;
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
    value parse_file(std::string aFilename);

      // ----------------------------------------------------------------------

    inline void object::insert(value&& aKey, value&& aValue)
    {
        mContent.emplace_back(std::get<rjson::string>(aKey), aValue);
    }

    inline value& object::get_ref(std::string aKey, value&& aDefault)
    {
        auto found = std::find_if(std::begin(mContent), std::end(mContent), [&aKey](const auto& entry) { return entry.first == aKey; });
        if (found == std::end(mContent)) {
            std::cerr << "object::get_ref: not found: " << aKey << ' ' << to_json() << '\n';
            return mContent.emplace_back(std::move(aKey), std::move(aDefault)).second;
        }
        else {
            std::cerr << "object::get_ref: found: " << aKey << ' ' << found->second.to_json() << '\n';
            return found->second;
        }
    }

      // ----------------------------------------------------------------------

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

inline std::string rjson::value::to_json() const
{
    return std::visit([](auto&& arg) -> std::string { return arg.to_json(); }, *this);
}

// ----------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& out, const rjson::value& aValue)
{
    return out << aValue.to_json();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
