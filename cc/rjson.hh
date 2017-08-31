#pragma once

#include <stdexcept>
#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <iostream>
#include <cassert>

#include "acmacs-base/debug.hh"
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
        inline size_t size() const { return mData.size(); }
        inline bool empty() const { return mData.empty(); }
        inline bool operator<(const string& to_compare) const { return mData < to_compare.mData; }

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
        inline number(double aSrc) : mValue{double_to_string(aSrc)} {}
        inline number& operator=(double aSrc) { mValue = double_to_string(aSrc); return *this; }
        inline std::string to_json() const { return mValue; } // { return double_to_string(mValue); }
        inline operator double() const { return std::stod(mValue); } // { return mValue; }

     private:
        inline number(std::string_view&& aData) : mValue{aData} {} // mValue{std::stod(static_cast<std::string>(aData))} {}

          // double mValue;
        std::string mValue;

        friend class implementation::NumberHandler;
    };

    class integer
    {
     public:
        inline integer(long aSrc) : mValue{std::to_string(aSrc)} {}
        inline integer& operator=(long aSrc) { mValue = std::to_string(aSrc); return *this; }
        inline std::string to_json() const { return mValue; } // { return std::to_string(mValue); }
        inline operator double() const { return std::stod(mValue); } // { return mValue; }
        inline operator long() const { return std::stol(mValue); } // { return mValue; }

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
        inline size_t size() const { return mContent.size(); }
        inline bool empty() const { return mContent.empty(); }

          // returns reference to the value at the passed key.
          // if key not found, inserts aDefault with the passed key and returns reference to the inserted
        value& get_ref(std::string aKey, value&& aDefault);
        void set_field(std::string aKey, value&& aValue);

     private:
        std::map<string, value> mContent;
          // std::vector<std::pair<string, value>> mContent;
    };

    class array
    {
     public:
        std::string to_json() const;

        void insert(value&& aValue);
        inline size_t size() const { return mContent.size(); }
        inline bool empty() const { return mContent.empty(); }
        inline value& operator[](size_t index) { return mContent[index]; }
        inline const value& operator[](size_t index) const { return mContent[index]; }

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

        template <typename FValue> value to_value(const FValue& aValue);
    }

    using value_base = std::variant<null, object, array, string, integer, number, boolean>; // null must be the first alternative, it is the default value;

    class value : public value_base
    {
     public:
        using value_base::operator=;
        using value_base::value_base;
        inline value(const value&) = default; // gcc7 wants this, otherwise it is deleted
        inline value& operator=(const value&) = default; // gcc7 wants this, otherwise it is deleted
          // inline ~value() { std::cerr << "DEBUG: ~value " << to_json() << DEBUG_LINE_FUNC << '\n'; }

          // returns reference to the value at the passed key.
          // if key not found, inserts aDefault with the passed key and returns reference to the inserted
          // if this is not an object, throws  std::bad_variant_access
        inline value& get_ref(std::string aKey, value&& aDefault)
            {
                try {
                    return std::get<object>(*this).get_ref(aKey, std::forward<value>(aDefault));
                }
                catch (std::bad_variant_access&) {
                    std::cerr << "ERROR: rjson::value::get_ref: not an object: valueless_by_exception:" << valueless_by_exception() << " index:" << index() << '\n'; // to_json() << '\n';
                    throw;
                }
            }

        template <typename F> inline std::decay_t<F> get_field(std::string aFieldName, F&& aDefaultValue)
            {
                using rjson_type = typename implementation::content_type<std::decay_t<F>>::type;
                return std::get<rjson_type>(get_ref(aFieldName, rjson_type{std::forward<F>(aDefaultValue)}));
            }

        template <typename F> inline void set_field(std::string aFieldName, F&& aValue)
            {
                try {
                    std::get<object>(*this).set_field(aFieldName, implementation::to_value(aValue));
                }
                catch (std::bad_variant_access&) {
                    std::cerr << "ERROR: rjson::value::set_field: not an object: " << to_json() << '\n';
                    throw;
                }
                  // using rjson_type = typename implementation::content_type<std::decay_t<F>>::type;
                  // std::get<object>(*this).set_field(aFieldName, rjson_type{std::forward<F>(aValue)});
            }

        std::string to_json() const;
    };

    namespace implementation
    {
        template <typename FValue> inline value to_value(const FValue& aValue)
        {
            using rjson_type = typename implementation::content_type<std::decay_t<FValue>>::type;
            return rjson_type{aValue};
        }
    }

      // ----------------------------------------------------------------------

    class field_container
    {
     public:
        inline void use_json(value& aData) { mData = &aData; }
        inline operator value&() { assert(mData); return *mData; }
        inline operator const value&() const { assert(mData); return *mData; }
        inline std::string to_json() const { return static_cast<const value&>(*this).to_json(); }

     private:
        mutable value* mData = nullptr;
    };

    template <typename FValue> class field_get_set
    {
     public:
        inline field_get_set(field_container* aContainer, std::string aFieldName, FValue&& aDefault) : mContainer{*aContainer}, mFieldName{aFieldName}, mDefault{std::move(aDefault)} {}
        inline operator FValue() const { return static_cast<value&>(mContainer).get_field(mFieldName, mDefault); } // static_cast to non-const because we need to set to mDefault
        inline field_get_set<FValue>& operator = (FValue&& aValue) { static_cast<value&>(mContainer).set_field(mFieldName, std::forward<FValue>(aValue)); return *this; }
        inline field_get_set<FValue>& operator = (const FValue& aValue) { static_cast<value&>(mContainer).set_field(mFieldName, aValue); return *this; }
        inline field_get_set<FValue>& operator = (const field_get_set<FValue>& aSource) { return operator=(static_cast<FValue>(aSource)); }

     protected:
        inline value& get_ref() { return static_cast<value&>(mContainer).get_ref(mFieldName, implementation::to_value(mDefault)); }
        inline const value& get_ref() const { return static_cast<value&>(mContainer).get_ref(mFieldName, implementation::to_value(mDefault)); }
        using rjson_type = typename implementation::content_type<FValue>::type;
        inline rjson_type& get_value_ref() { return std::get<rjson_type>(get_ref()); }
        inline const rjson_type& get_value_ref() const { return std::get<rjson_type>(get_ref()); }

     private:
        field_container& mContainer;
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
        mContent.emplace(std::get<rjson::string>(aKey), aValue);
          // mContent.emplace_back(std::get<rjson::string>(aKey), aValue);
    }

    inline value& object::get_ref(std::string aKey, value&& aDefault)
    {
        const auto [iter, inserted] = mContent.emplace(aKey, std::forward<value>(aDefault));
        return iter->second;

        // auto found = std::find_if(std::begin(mContent), std::end(mContent), [&aKey](const auto& entry) { return entry.first == aKey; });
        // if (found == std::end(mContent)) {
        //       // std::cerr << "DEBUG: object::get_ref: not found: " << aKey << ' ' << to_json() << " default:" << aDefault.to_json() << '\n';
        //       return mContent.emplace_back(aKey, std::forward<value>(aDefault)).second;
        // }
        // else {
        //     // std::cerr << "DEBUG: object::get_ref: found: " << aKey << ' ' << found->second.to_json() << '\n';
        //     return found->second;
        // }
    }

    inline void object::set_field(std::string aKey, value&& aValue)
    {
        mContent.insert_or_assign(aKey, std::forward<value>(aValue));

        // auto found = std::find_if(std::begin(mContent), std::end(mContent), [&aKey](const auto& entry) { return entry.first == aKey; });
        // if (found == std::end(mContent)) {
        //     mContent.emplace_back(std::move(aKey), std::forward<value>(aValue));
        // }
        // else {
        //     found->second = std::forward<value>(aValue);
        // }
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
