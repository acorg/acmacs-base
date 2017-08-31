#pragma once

#include <stdexcept>
#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>

#include "acmacs-base/float.hh"
#include "acmacs-base/filesystem.hh"

// ----------------------------------------------------------------------

namespace rjson
{
    namespace implementation { class NumberHandler; }

    class value;

    class string
    {
     public:
        inline string(std::string aData) : mData{aData} {}
        inline string(const char* aData) : mData{aData} {}
        inline std::string to_json() const { return std::string{"\""} + static_cast<std::string>(mData) + "\""; }
        inline operator std::string() const { return mData; }
        inline string& operator=(std::string aSrc) { mData = aSrc; return *this; }
        inline bool operator==(const std::string aToCompare) const { return mData == aToCompare; }
        inline bool operator==(const string& aToCompare) const { return mData == aToCompare.mData; }

     private:
        std::string mData;
    };

    class boolean
    {
     public:
        inline boolean(bool aValue) : mValue{aValue} {}
        inline std::string to_json() const { return mValue ? "true" : "false"; }
        inline operator bool() const { return mValue; }
        inline boolean& operator=(bool aSrc) { mValue = aSrc; return *this; }

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
        inline std::string to_json() const { return mValue; } // { return double_to_string(mValue); }
        inline operator double() const { return std::stod(mValue); } // { return mValue; }
        inline number& operator=(double aSrc) { mValue = double_to_string(aSrc); return *this; }

     private:
        inline number(std::string_view&& aData) : mValue{aData} {} // mValue{std::stod(static_cast<std::string>(aData))} {}

          // double mValue;
        std::string mValue;

        friend class implementation::NumberHandler;
    };

    class integer
    {
     public:
        inline std::string to_json() const { return mValue; } // { return std::to_string(mValue); }
        inline operator double() const { return std::stod(mValue); } // { return mValue; }
        inline operator long() const { return std::stol(mValue); } // { return mValue; }
        inline integer& operator=(long aSrc) { mValue = std::to_string(aSrc); return *this; }

     private:
        inline integer(std::string_view&& aData) : mValue{aData} {} // mValue{std::stol(static_cast<std::string>(aData))} {}

          // long mValue;
        std::string mValue;

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
        void set_field(std::string aKey, value&& aValue);

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

    namespace implementation
    {
        template <typename F> struct content_type;
        template <> struct content_type<double> { using type = rjson::number; };
        template <> struct content_type<long> { using type = rjson::integer; };
        template <> struct content_type<int> { using type = rjson::integer; };
        template <> struct content_type<bool> { using type = rjson::boolean; };
        template <> struct content_type<std::string> { using type = rjson::string; };
    }

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

        template <typename F> inline std::decay_t<F> get_field(std::string aFieldName, F&& aDefaultValue)
            {
                using type = typename implementation::content_type<std::decay_t<F>>::type;
                return std::get<type>(get_ref(aFieldName, std::forward<value>(aDefaultValue)));
            }

        template <typename F> inline void set_field(std::string aFieldName, F&& aValue)
            {
                std::get<object>(*this).set_field(aFieldName, std::forward<value>(aValue));
            }

        std::string to_json() const;
    };

      // ----------------------------------------------------------------------

    template <typename FValue> class field_get_set
    {
     public:
        inline field_get_set(std::string aFieldName, FValue&& aDefault) : mFieldName{aFieldName}, mDefault{std::move(aDefault)} {}
        inline operator FValue() const { return mContainer->get_field(mFieldName, mDefault); }
        inline void operator = (FValue&& aValue) { mContainer->set_field(mFieldName, std::forward<FValue>(aValue)); }
        inline void container(value* aContainer) { mContainer = aContainer; }

     private:
        mutable value* mContainer = nullptr;
        std::string mFieldName;
        FValue mDefault;
    };

      // ----------------------------------------------------------------------

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

    value parse_string(std::string aJsonData);
    value parse_file(std::string aFilename);

} // namespace rjson

// ----------------------------------------------------------------------
// gcc-7 support
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
// inline
// ----------------------------------------------------------------------

namespace rjson
{
    inline void object::insert(value&& aKey, value&& aValue)
    {
        mContent.emplace_back(std::get<rjson::string>(aKey), aValue);
    }

    inline value& object::get_ref(std::string aKey, value&& aDefault)
    {
        auto found = std::find_if(std::begin(mContent), std::end(mContent), [&aKey](const auto& entry) { return entry.first == aKey; });
        if (found == std::end(mContent)) {
              // std::cerr << "DEBUG: object::get_ref: not found: " << aKey << ' ' << to_json() << " default:" << aDefault.to_json() << '\n';
            return mContent.emplace_back(std::move(aKey), std::forward<value>(aDefault)).second;
        }
        else {
              // std::d::cerr << "DEBUG: object::get_ref: found: " << aKey << ' ' << found->second.to_json() << '\n';
            return found->second;
        }
    }

    inline void object::set_field(std::string aKey, value&& aValue)
    {
        auto found = std::find_if(std::begin(mContent), std::end(mContent), [&aKey](const auto& entry) { return entry.first == aKey; });
        if (found == std::end(mContent)) {
            mContent.emplace_back(std::move(aKey), std::forward<value>(aValue));
        }
        else {
            found->second = std::forward<value>(aValue);
        }
    }

      // ----------------------------------------------------------------------

    inline void array::insert(value&& aValue)
    {
        mContent.push_back(std::move(aValue));
    }

      // ----------------------------------------------------------------------

    inline std::string value::to_json() const
    {
        return std::visit([](auto&& arg) -> std::string { return arg.to_json(); }, *this);
    }
}

// ----------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& out, const rjson::value& aValue)
{
    return out << aValue.to_json();
}

template <typename FValue> inline std::ostream& operator<<(std::ostream& out, const rjson::field_get_set<FValue>& aValue)
{
    return out << static_cast<FValue>(aValue);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
