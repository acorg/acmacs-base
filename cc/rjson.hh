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
#include "acmacs-base/to-string.hh"
#include "acmacs-base/filesystem.hh"

// ----------------------------------------------------------------------

namespace rjson
{
    class field_not_found : public std::runtime_error { public: using std::runtime_error::runtime_error; inline field_not_found() : std::runtime_error{""} {} };

    class value;
    class array;
    class object;

    extern value sNull;
    extern value sEmptyArray;
    extern value sEmptyObject;

    enum class json_pp_emacs_indent { no, yes };

    template <typename F> struct content_type;
    template <typename F> using rjson_type = typename content_type<std::decay_t<F>>::type;

    class string
    {
     public:
        inline string() = default;
        inline string(std::string aData) : mData{aData} {}
        inline string(const char* aData) : mData{aData} {}
        inline string(const string&) = default;
        inline string& operator=(const string&) = default;
        inline string(string&&) = default;
        inline string& operator=(string&&) = default;
        inline std::string to_json() const { return std::string{"\""} + static_cast<std::string>(mData) + "\""; }
        inline std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
        inline operator std::string() const { return mData; }
        inline string& operator=(std::string aSrc) { mData = aSrc; return *this; }
        inline bool operator==(const std::string aToCompare) const { return mData == aToCompare; }
        inline bool operator==(const string& aToCompare) const { return mData == aToCompare.mData; }
        inline bool operator!=(const string& aToCompare) const { return ! operator==(aToCompare); }
        inline bool operator==(const char* aToCompare) const { return mData == aToCompare; }
        inline bool operator!=(const char* aToCompare) const { return ! operator==(aToCompare); }
        inline size_t size() const { return mData.size(); }
        inline bool empty() const { return mData.empty(); }
        inline char front() const { return mData.front(); }
        inline char back() const { return mData.back(); }
        inline bool operator<(const string& to_compare) const { return mData < to_compare.mData; }
        inline void update(const string& to_merge) { mData = to_merge.mData; }
        inline void remove_comments() {}
        template <typename Index> [[noreturn]] inline const value& operator[](Index) const { throw field_not_found{};}
        template <typename Index> [[noreturn]] inline value& operator[](Index) { throw field_not_found{}; }
        template <typename T> [[noreturn]] inline value& get_or_add(std::string, T&&) { throw field_not_found{}; }

     private:
        std::string mData;

        inline bool is_comment_key() const { return !mData.empty() && (mData.front() == '?' || mData.back() == '?'); }

        friend class object;

    }; // class string

    class boolean
    {
     public:
        inline boolean() : mValue{false} {}
        inline boolean(bool aValue) : mValue{aValue} {}
        inline std::string to_json() const { return mValue ? "true" : "false"; }
        inline std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
        inline operator bool() const { return mValue; }
        inline boolean& operator=(bool aSrc) { mValue = aSrc; return *this; }
        inline void update(const boolean& to_merge) { mValue = to_merge.mValue; }
        inline void remove_comments() {}
        template <typename Index> [[noreturn]] inline const value& operator[](Index) const { throw field_not_found{}; }
        template <typename Index> [[noreturn]] inline value& operator[](Index) { throw field_not_found{}; }
        template <typename T> [[noreturn]] inline value& get_or_add(std::string, T&&) { throw field_not_found{}; }

     private:
        bool mValue;

    }; // class boolean

    class null
    {
     public:
        inline null() {}
        inline void update(const null&) {}
        inline std::string to_json() const { return "null"; }
        inline std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
        inline void remove_comments() {}
        template <typename Index> [[noreturn]] inline const value& operator[](Index) const { throw field_not_found{}; }
        template <typename Index> [[noreturn]] inline value& operator[](Index) { throw field_not_found{}; }
        template <typename T> [[noreturn]] inline value& get_or_add(std::string, T&&) { throw field_not_found{}; }

    }; // class null

    class number
    {
     public:
        inline number() : mValue{"0.0"} {}
        inline number(double aSrc) : mValue{acmacs::to_string(aSrc)} {}
        inline number(std::string_view&& aData) : mValue{aData} {} // for parser
        inline number& operator=(double aSrc) { mValue = acmacs::to_string(aSrc); return *this; }
        inline std::string to_json() const { return mValue; }
        inline std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
        inline operator double() const { return std::stod(mValue); }
        inline void update(const number& to_merge) { mValue = to_merge.mValue; }
        inline void remove_comments() {}
        template <typename Index> [[noreturn]] inline const value& operator[](Index) const { throw field_not_found{}; }
        template <typename Index> [[noreturn]] inline value& operator[](Index) { throw field_not_found{}; }
        template <typename T> [[noreturn]] inline value& get_or_add(std::string, T&&) { throw field_not_found{}; }

     private:
          // double mValue;
        std::string mValue;

    }; // class number

    class integer
    {
     public:
        inline integer() : mValue{"0"} {}
        inline integer(int aSrc) : mValue{acmacs::to_string(aSrc)} {}
        inline integer(unsigned int aSrc) : mValue{acmacs::to_string(aSrc)} {}
        inline integer(long aSrc) : mValue{acmacs::to_string(aSrc)} {}
        inline integer(unsigned long aSrc) : mValue{acmacs::to_string(aSrc)} {}
        inline integer(std::string_view&& aData) : mValue{aData} {} // for parser
        inline integer& operator=(int aSrc) { mValue = acmacs::to_string(aSrc); return *this; }
        inline integer& operator=(unsigned int aSrc) { mValue = acmacs::to_string(aSrc); return *this; }
        inline integer& operator=(long aSrc) { mValue = acmacs::to_string(aSrc); return *this; }
        inline integer& operator=(unsigned long aSrc) { mValue = acmacs::to_string(aSrc); return *this; }
        inline std::string to_json() const { return mValue; }
        inline std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
        inline operator double() const { return std::stod(mValue); }
        inline operator long() const { return std::stol(mValue); }
        inline operator unsigned long() const { return std::stoul(mValue); }
        inline operator int() const { return static_cast<int>(std::stol(mValue)); }
        inline operator unsigned int() const { return static_cast<unsigned int>(std::stoul(mValue)); }
        inline void update(const integer& to_merge) { mValue = to_merge.mValue; }
        inline void remove_comments() {}
        template <typename Index> [[noreturn]] inline const value& operator[](Index) const { throw field_not_found{}; }
        template <typename Index> [[noreturn]] inline value& operator[](Index) { throw field_not_found{}; }
        template <typename T> [[noreturn]] inline value& get_or_add(std::string, T&&) { throw field_not_found{}; }

     private:
        std::string mValue;

    }; // class integer

    class object
    {
     public:
        inline object() = default;
        inline object(std::initializer_list<std::pair<string, value>> key_values);
        // inline ~object() { std::cerr << "~rjson::object " << to_json(true) << '\n'; }
        inline object(const object&) = default; // required if explicit destructor provided
        inline object& operator=(const object&) = default; // required if explicit destructor provided
        inline object(object&&) = default;
        inline object& operator=(object&&) = default;

        std::string to_json(bool space_after_comma = false) const;
        std::string to_json_pp(size_t indent, json_pp_emacs_indent emacs_indent = json_pp_emacs_indent::no, size_t prefix = 0) const;

        void insert(const string& aKey, value&& aValue);
        void insert(const string& aKey, const value& aValue);
        void insert(value&& aKey, value&& aValue);
        inline size_t size() const { return mContent.size(); }
        inline bool empty() const { return mContent.empty(); }

          // if field is not in the object, throws field_not_found
        const value& operator[](std::string aFieldName) const;
        value& operator[](std::string aFieldName);
        inline value& operator[](const rjson::string& aFieldName) { return operator[](static_cast<std::string>(aFieldName)); }
        inline value& operator[](const char* aFieldName) { return operator[](std::string{aFieldName}); }
        inline const value& operator[](const char* aFieldName) const { return operator[](std::string{aFieldName}); }
        template <typename Index> [[noreturn]] inline const value& operator[](Index) const { throw field_not_found{}; }
        template <typename Index> [[noreturn]] inline value& operator[](Index) { throw field_not_found{}; }
        template <typename T> value& get_or_add(std::string aFieldName, T&& aDefault);

        template <typename T> inline std::decay_t<T> get_or_default(std::string aFieldName, T&& aDefault) const
            {
                static_assert(!std::is_same_v<T, rjson::object> && !std::is_same_v<T, rjson::array>, "get_or_default returns a copy, not a reference, use get_or_empty_object or get_or_empty_array");
                try {
                    return operator[](aFieldName);
                }
                catch (field_not_found&) {
                    return std::forward<T>(aDefault);
                }
            }

        inline std::string get_or_default(std::string aFieldName, const char* aDefault) const
            {
                return get_or_default<std::string>(aFieldName, aDefault);
            }

        template <typename R> std::pair<bool, const R&> get_R_if(std::string aFieldName) const;
        bool exists(std::string aFieldName) const;
        std::pair<bool, const value&> get_value_if(std::string aFieldName) const;
        std::pair<bool, const object&> get_object_if(std::string aFieldName) const;
        std::pair<bool, const array&> get_array_if(std::string aFieldName) const;
        const object& get_or_empty_object(std::string aFieldName) const;
        const array& get_or_empty_array(std::string aFieldName) const;

        value& set_field(std::string aKey, value&& aValue); // returns ref to inserted
        value& set_field(const string& aKey, const value& aValue); // returns ref to inserted
        template <typename T> void set_field_if_not_empty(std::string aKey, const T& aValue);
        template <typename T> void set_field_if_not_default(std::string aKey, const T& aValue, const T& aDefault);
        template <typename Iterator> void set_array_field_if_not_empty(std::string aKey, Iterator first, Iterator last);
        template <typename Container> inline void set_array_field_if_not_empty(std::string aKey, const Container& aContainer) { set_array_field_if_not_empty(aKey, std::begin(aContainer), std::end(aContainer)); }

        void delete_field(string aKey); // throws field_not_found
        inline void clear() { mContent.clear(); }

        using const_iterator = decltype(std::declval<std::map<string, value>>().cbegin());
        inline const_iterator begin() const { return mContent.begin(); }
        inline const_iterator end() const { return mContent.end(); }
        using iterator = decltype(std::declval<std::map<string, value>>().begin());
        inline iterator begin() { return mContent.begin(); }
        inline iterator end() { return mContent.end(); }

        void update(const object& to_merge);
        void remove_comments();

        static constexpr const char* const force_pp_key = "**rjson_pp**";
        static constexpr const char* const no_pp_key = "**rjson_no_pp**";

     private:
        std::map<string, value> mContent;

    }; // class object

    class array
    {
     public:
        inline array() = default;
        inline array(array&&) = default;
        inline array(const array&) = default;
        template <typename ... Args> array(Args ... args);
        inline array& operator=(array&&) = default;
        inline array& operator=(const array&) = default;
        inline value& operator[](size_t index) { return mContent.at(index); }
        inline value& operator[](int index) { return mContent.at(static_cast<decltype(mContent)::size_type>(index)); }
        template <typename Index> [[noreturn]] inline const value& operator[](Index) const { throw field_not_found{}; }
        template <typename Index> [[noreturn]] inline value& operator[](Index) { throw field_not_found{}; }
        template <typename T> [[noreturn]] inline value& get_or_add(std::string, T&&) { throw field_not_found{}; }

        inline void update(const array& to_merge) { mContent = to_merge.mContent; } // replace content!
        void remove_comments();
        std::string to_json(bool space_after_comma = false) const;
        std::string to_json_pp(size_t indent, json_pp_emacs_indent emacs_indent = json_pp_emacs_indent::no, size_t prefix = 0) const;

        value& insert(value&& aValue); // returns ref to inserted
        value& insert(const value& aValue); // returns ref to inserted
        inline size_t size() const { return mContent.size(); }
        inline bool empty() const { return mContent.empty(); }
        inline const value& operator[](size_t index) const { return mContent.at(index); }
        inline const value& operator[](int index) const { return mContent.at(static_cast<decltype(mContent)::size_type>(index)); }
        inline void erase(size_t index) { mContent.erase(mContent.begin() + static_cast<std::vector<value>::difference_type>(index)); }
        inline void erase(int index) { mContent.erase(mContent.begin() + index); }
        inline void clear() { mContent.clear(); }

        using iterator = decltype(std::declval<const std::vector<value>>().begin());
        using reverse_iterator = decltype(std::declval<const std::vector<value>>().rbegin());
        inline iterator begin() const { return mContent.begin(); }
        inline iterator end() const { return mContent.end(); }
        inline iterator begin() { return mContent.begin(); }
        inline iterator end() { return mContent.end(); }
        inline reverse_iterator rbegin() const { return mContent.rbegin(); }

     private:
        std::vector<value> mContent;

    }; // class array

      // ----------------------------------------------------------------------

    template <> struct content_type<double> { using type = rjson::number; };
    template <> struct content_type<long> { using type = rjson::integer; };
    template <> struct content_type<unsigned long> { using type = rjson::integer; };
    template <> struct content_type<int> { using type = rjson::integer; };
    template <> struct content_type<unsigned int> { using type = rjson::integer; };
    template <> struct content_type<bool> { using type = rjson::boolean; };
    template <> struct content_type<std::string> { using type = rjson::string; };
    template <> struct content_type<char*> { using type = rjson::string; };
    template <> struct content_type<const char*> { using type = rjson::string; };

    template <> struct content_type<null> { using type = rjson::null; };
    template <> struct content_type<object> { using type = rjson::object; };
    template <> struct content_type<array> { using type = rjson::array; };
    template <> struct content_type<string> { using type = rjson::string; };
    template <> struct content_type<boolean> { using type = rjson::boolean; };
    template <> struct content_type<number> { using type = rjson::number; };
    template <> struct content_type<integer> { using type = rjson::integer; };

    template <typename FValue> value to_value(const FValue& aValue);
    template <typename FValue> value to_value(FValue&& aValue);

      // ----------------------------------------------------------------------

    using value_base = std::variant<null, object, array, string, integer, number, boolean>; // null must be the first alternative, it is the default value;

    class value : public value_base
    {
     public:
        using value_base::operator=;
        using value_base::value_base;
        inline value(const value&) = default; // otherwise it is deleted
          //inline value(const value& aSrc) : value_base(aSrc) { std::cerr << "rjson::value copy " << aSrc.to_json() << '\n'; }
        inline value(value&&) = default;
          // inline value(value&& aSrc) : value_base(std::move(aSrc)) { std::cerr << "rjson::value move " << to_json() << '\n'; }
        inline value& operator=(const value&) = default; // otherwise it is deleted
        inline value& operator=(value&&) = default;
          // inline ~value() { std::cerr << "DEBUG: ~value " << to_json() << DEBUG_LINE_FUNC << '\n'; }

          // ----------------------------------------------------------------------

        inline operator unsigned long() const { return std::get<integer>(*this); }
        inline operator long() const { return std::get<integer>(*this); }
        inline operator unsigned int() const { return std::get<integer>(*this); }
        inline operator int() const { return std::get<integer>(*this); }
        inline operator bool() const { return std::get<boolean>(*this); }
        inline operator std::string() const { return std::get<string>(*this); }

        inline operator double() const
            {
                if (auto ptr_n = std::get_if<number>(this))
                    return *ptr_n;
                else if (auto ptr_i = std::get_if<integer>(this))
                    return *ptr_i;
                else {
                    std::cerr << "ERROR: cannot convert json to double (from rjson::number or rjson::integer): " << to_json() << '\n';
                    throw std::bad_variant_access{};
                }
            }

        inline operator const null&() const { return std::get<null>(*this); }
        inline operator const boolean&() const { return std::get<boolean>(*this); }
        inline operator const string&() const { return std::get<string>(*this); }
        inline operator const integer&() const { return std::get<integer>(*this); }
        inline operator const number&() const { return std::get<number>(*this); }
        inline operator const object&() const { return std::get<object>(*this); }
        inline operator const array&() const { return std::get<array>(*this); }
        inline operator object&() { return std::get<object>(*this); }
        inline operator array&() { return std::get<array>(*this); }

        bool empty() const;

          // ----------------------------------------------------------------------

        template <typename Index> const value& operator[](Index aIndex) const;

        template <typename Index> inline value& operator[](Index aIndex)
            {
                return std::visit([&](auto&& arg) -> value& { return arg[aIndex]; }, *this);
            }

        template <typename T> T get_or_default(std::string aFieldName, T&& aDefault) const;

        inline std::string get_or_default(std::string aFieldName, const char* aDefault) const
            {
                return get_or_default<std::string>(aFieldName, aDefault);
            }

        const object& get_or_empty_object(std::string aFieldName) const;
        const array& get_or_empty_array(std::string aFieldName) const;

        bool exists(std::string aFieldName) const
            {
                try {
                    operator[](aFieldName);
                    return true;
                }
                catch (field_not_found&) {
                    return false;
                }
            }

        template <typename R> inline std::pair<bool, const R&> get_R_if(std::string aFieldName) const
            {
                try {
                    return {true, operator[](aFieldName)};
                }
                catch (field_not_found&) {
                    if constexpr (std::is_same_v<R, value>)
                        return {false, sNull};
                    else if constexpr (std::is_same_v<R, object>)
                        return {false, sEmptyObject};
                    else
                        return {false, sEmptyArray};
                }
            }

        std::pair<bool, const value&> inline get_value_if(std::string aFieldName) const { return get_R_if<value>(aFieldName); }
        std::pair<bool, const object&> inline get_object_if(std::string aFieldName) const { return get_R_if<object>(aFieldName); }
        std::pair<bool, const array&> inline get_array_if(std::string aFieldName) const { return get_R_if<array>(aFieldName); }

        template <typename T> inline value& get_or_add(std::string aFieldName, T&& aDefault)
            {
                return std::visit([&](auto&& arg) -> value& { return arg.get_or_add(aFieldName, std::forward<T>(aDefault)); }, *this);
            }

        template <typename T> inline value& get_or_add(std::string aFieldName, const T& aDefault)
            {
                T move_default{aDefault};
                return std::visit([&](auto&& arg) -> value& { return arg.get_or_add(aFieldName, std::forward<T>(move_default)); }, *this);
            }

        template <typename F> inline void set_field(std::string aFieldName, F&& aValue)
            {
                return std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, object>)
                        arg.set_field(aFieldName, std::forward<F>(aValue));
                    else
                        throw field_not_found{aFieldName};
                }, *this);
            }

        template <typename F> inline void set_field(std::string aFieldName, const F& aValue)
            {
                return std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, object>)
                        arg.set_field(aFieldName, aValue);
                    else
                        throw field_not_found{aFieldName};
                }, *this);
            }

        void delete_field(std::string aFieldName);

        value& update(const value& to_merge);
        void remove_comments(); // defined below as inline (gcc 7.2 cannot handle it inlined here)

        std::string to_json() const;
        std::string to_json_pp(size_t indent = 2, json_pp_emacs_indent emacs_indent = json_pp_emacs_indent::yes, size_t prefix = 0) const;

    }; // class value

      // ----------------------------------------------------------------------

    template <> struct content_type<value> { using type = value; };

    template <typename FValue> inline value to_value(const FValue& aValue)
    {
          // return rjson_type<std::decay_t<FValue>>{aValue};
        return rjson_type<FValue>{aValue};
    }

    template <typename FValue> inline value to_value(FValue&& aValue)
    {
          // return rjson_type<std::decay_t<FValue>>{std::forward<FValue>(aValue)};
        return rjson_type<FValue>{std::forward<FValue>(aValue)};
    }

    // template <typename FValue> inline value to_value(value&& aValue) { return aValue; }
    template <typename FValue> inline value to_value(object&& aValue) { return aValue; }
    template <typename FValue> inline value to_value(array&& aValue) { return aValue; }

      // template <typename FValue> inline value to_value(const value& aValue) { return aValue; }
    template <typename FValue> inline value to_value(const object& aValue) { return aValue; }
    template <typename FValue> inline value to_value(const array& aValue) { return aValue; }

      // ----------------------------------------------------------------------

    template <typename Result> class value_visitor_base
    {
     public:
        class unexpected_value : public std::runtime_error { public: using std::runtime_error::runtime_error; };

        inline value_visitor_base() = default;
        virtual inline ~value_visitor_base() {}

        [[noreturn]] virtual inline Result operator()(null& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
        [[noreturn]] virtual inline Result operator()(object& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
        [[noreturn]] virtual inline Result operator()(array& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
        [[noreturn]] virtual inline Result operator()(string& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
        [[noreturn]] virtual inline Result operator()(integer& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
        [[noreturn]] virtual inline Result operator()(number& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
        [[noreturn]] virtual inline Result operator()(boolean& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }

        [[noreturn]] virtual inline Result operator()(const rjson::null& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
        [[noreturn]] virtual inline Result operator()(const rjson::object& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
        [[noreturn]] virtual inline Result operator()(const rjson::array& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
        [[noreturn]] virtual inline Result operator()(const rjson::string& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
        [[noreturn]] virtual inline Result operator()(const rjson::integer& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
        [[noreturn]] virtual inline Result operator()(const rjson::number& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
        [[noreturn]] virtual inline Result operator()(const rjson::boolean& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }

     protected:
        [[noreturn]] virtual inline void throw_unexpected_value(std::string aMessage) { throw unexpected_value{aMessage}; }

    }; // class value_visitor_base

      // ----------------------------------------------------------------------

    class parse_error : public std::exception
    {
     public:
        inline parse_error(size_t aLine, size_t aColumn, std::string&& aMessage)
            : mMessage{acmacs::to_string(aLine) + ":" + acmacs::to_string(aColumn) + ": " + std::move(aMessage)} //, mLine{aLine}, mColumn{aColumn}
            {}

        inline const char* what() const noexcept override { return mMessage.c_str(); }

     private:
        std::string mMessage;
          //size_t mLine, mColumn;

    }; // class parse_error

    class merge_error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    enum class remove_comments { No, Yes };

    value parse_string(std::string aJsonData, remove_comments aRemoveComments = remove_comments::Yes);
    value parse_string(const std::string_view& aJsonData, remove_comments aRemoveComments = remove_comments::Yes);
    value parse_string(const char* aJsonData, remove_comments aRemoveComments = remove_comments::Yes);
    value parse_file(std::string aFilename, remove_comments aRemoveComments = remove_comments::Yes);

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
    inline void object::insert(const string& aKey, value&& aValue) { mContent.emplace(aKey, std::move(aValue)); }
    inline void object::insert(const string& aKey, const value& aValue) { mContent.emplace(aKey, aValue); }
    inline void object::insert(value&& aKey, value&& aValue) { insert(std::get<string>(std::move(aKey)), std::move(aValue)); }

    inline object::object(std::initializer_list<std::pair<string, value>> key_values)
    {
        for (const auto& [key, value]: key_values)
            insert(key, value);
    }

    inline const value& object::operator[](std::string aFieldName) const
    {
        if (const auto existing = mContent.find(aFieldName); existing != mContent.end())
            return existing->second;
        else
            throw field_not_found{aFieldName};
    }

    inline value& object::operator[](std::string aFieldName)
    {
        if (const auto existing = mContent.find(aFieldName); existing != mContent.end())
            return existing->second;
        else
            throw field_not_found{aFieldName};
    }

    template <> inline value& object::get_or_add(std::string aFieldName, value&& aDefault)
    {
        try {
            return operator[](aFieldName);
        }
        catch (field_not_found&) {
            return set_field(aFieldName, std::move(aDefault));
        }
    }

    template <typename T> inline value& get_or_add(std::string aFieldName, T&& aDefault)
    {
        return get_or_add(aFieldName, to_value(std::forward<T>(aDefault)));
    }

    inline value& object::set_field(std::string aKey, value&& aValue)
    {
        return mContent.insert_or_assign(aKey, std::forward<value>(aValue)).first->second;
    }

    inline value& object::set_field(const string& aKey, const value& aValue)
    {
        return mContent.insert_or_assign(aKey, aValue).first->second;
    }

    template <typename T> inline void object::set_field_if_not_empty(std::string aKey, const T& aValue)
    {
        if (!aValue.empty())
            set_field(aKey, to_value(aValue));
    }

    template <typename T> inline void object::set_field_if_not_default(std::string aKey, const T& aValue, const T& aDefault)
    {
        if (aValue != aDefault)
            set_field(aKey, to_value(aValue));
    }

    template <typename Iterator> inline void object::set_array_field_if_not_empty(std::string aKey, Iterator first, Iterator last)
    {
        if (first != last) {
            array& ar = set_field(aKey, array{});
            for (; first != last; ++first)
                ar.insert(rjson::to_value(*first));
        }

    } // object::set_array_field_if_not_empty

    inline void object::delete_field(string aKey)
    {
        if (auto iter = mContent.find(aKey); iter != mContent.end())
            mContent.erase(iter);
        else
            throw field_not_found{aKey};
    }

    inline const object& object::get_or_empty_object(std::string aFieldName) const
    {
        try {
            return operator[](aFieldName);
        }
        catch (field_not_found&) {
            return rjson::sEmptyObject;
        }
    }

    inline const array& object::get_or_empty_array(std::string aFieldName) const
    {
        try {
            return operator[](aFieldName);
        }
        catch (field_not_found&) {
            return rjson::sEmptyArray;
        }
    }

    inline bool object::exists(std::string aFieldName) const
    {
        try {
            operator[](aFieldName);
            return true;
        }
        catch (field_not_found&) {
            return false;
        }
    }

    template <typename R> inline std::pair<bool, const R&> object::get_R_if(std::string aFieldName) const
    {
        try {
            return {true, operator[](aFieldName)};
        }
        catch (field_not_found&) {
            if constexpr (std::is_same_v<R, value>)
                return {false, sNull};
            else if constexpr (std::is_same_v<R, object>)
                return {false, sEmptyObject};
            else
                return {false, sEmptyArray};
        }
    }

    std::pair<bool, const value&> inline object::get_value_if(std::string aFieldName) const { return get_R_if<value>(aFieldName); }
    std::pair<bool, const object&> inline object::get_object_if(std::string aFieldName) const { return get_R_if<object>(aFieldName); }
    std::pair<bool, const array&> inline object::get_array_if(std::string aFieldName) const { return get_R_if<array>(aFieldName); }

      // ----------------------------------------------------------------------

    inline value& array::insert(value&& aValue) { mContent.push_back(std::move(aValue)); return mContent.back(); }
    inline value& array::insert(const value& aValue) { mContent.push_back(aValue); return mContent.back(); }

    template <typename ... Args> inline array::array(Args ... args)
    {
        (insert(to_value(args)), ...);
    }

      // ----------------------------------------------------------------------

      // gcc 7.2 wants the following functions to be defined here (not inside the class

    inline bool value::empty() const
    {
        return std::visit([&](auto&& arg) -> bool {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, object> || std::is_same_v<T, array>)
                                 return arg.empty();
            else
                throw std::bad_variant_access{};
        }, *this);
    }

    inline void value::delete_field(std::string aFieldName)
    {
        return std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, object>)
                                 arg.delete_field(aFieldName);
            else
                throw field_not_found{aFieldName};
        }, *this);
    }

    template <typename Index> inline const value& value::operator[](Index aIndex) const
    {
        return std::visit([&](auto&& arg) -> const value& { return arg[aIndex]; }, *this);
    }

    template <typename T> inline T value::get_or_default(std::string aFieldName, T&& aDefault) const
    {
        static_assert(!std::is_same_v<T, rjson::object> && !std::is_same_v<T, rjson::array>, "get_or_default returns a copy, not a reference, use get_or_empty_object or get_or_empty_array");
        try {
            return operator[](aFieldName);
        }
        catch (field_not_found&) {
            return std::forward<T>(aDefault);
        }
    }

    inline const object& value::get_or_empty_object(std::string aFieldName) const
    {
        try {
            return operator[](aFieldName);
        }
        catch (field_not_found&) {
            return rjson::sEmptyObject;
        }
    }

    inline const array& value::get_or_empty_array(std::string aFieldName) const
    {
        try {
            return operator[](aFieldName);
        }
        catch (field_not_found&) {
            return rjson::sEmptyArray;
        }
    }

    template <> inline void value::set_field(std::string aFieldName, value&& aValue)
    {
        try {
            std::get<object>(*this).set_field(aFieldName, std::move(aValue));
        }
        catch (std::bad_variant_access&) {
            std::cerr << "ERROR: rjson::value::set_field: not an object: " << to_json() << '\n';
            throw;
        }
    }

    inline void value::remove_comments()
    {
        std::visit([](auto&& arg) -> void { arg.remove_comments(); }, *this);
    }

    inline std::string value::to_json() const
    {
        return std::visit([](auto&& arg) -> std::string { return arg.to_json(); }, *this);
    }

    inline std::string value::to_json_pp(size_t indent, json_pp_emacs_indent emacs_indent, size_t prefix) const
    {
        return std::visit([&](auto&& arg) -> std::string { return arg.to_json_pp(indent, emacs_indent, prefix); }, *this);
    }
}

// ----------------------------------------------------------------------

namespace ad_sfinae
{
    template <typename T, typename = void> struct has_to_json : std::false_type { };
    template <typename T> struct has_to_json<T, std::void_t<decltype(std::declval<const T>().to_json())>> : std::true_type { };
}

template <typename T> inline typename std::enable_if<ad_sfinae::has_to_json<T>::value, std::ostream&>::type operator<<(std::ostream& out, const T& aValue)
{
    return out << aValue.to_json();
}

// ----------------------------------------------------------------------

namespace acmacs
{
    inline std::string to_string(const rjson::string& src) { return src; }
    inline std::string to_string(const rjson::null&) { return "null"; }
    inline std::string to_string(const rjson::object& src) { return src.to_json(); }
    inline std::string to_string(const rjson::array& src) { return src.to_json(); }
    inline std::string to_string(const rjson::integer& src) { return src.to_json(); }
    inline std::string to_string(const rjson::number& src) { return src.to_json(); }
    inline std::string to_string(const rjson::boolean& src) { return src.to_json(); }

    inline std::string to_string(const rjson::value& src)
    {
        return std::visit([](auto&& arg) -> std::string { return acmacs::to_string(arg); }, src);
    }

}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
