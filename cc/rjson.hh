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
    class object;

    enum class json_pp_emacs_indent { no, yes };

    template <typename F> struct content_type;
    template <typename F> using rjson_type = typename content_type<std::decay_t<F>>::type;

    class string
    {
     public:
        inline string() = default;
        inline string(std::string aData) : mData{aData} {}
        inline string(const char* aData) : mData{aData} {}
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
        inline bool operator<(const string& to_compare) const { return mData < to_compare.mData; }
        inline void update(const string& to_merge) { mData = to_merge.mData; }
        inline void remove_comments() {}

     private:
        std::string mData;

        inline bool is_comment_key() const { return !mData.empty() && (mData.front() == '?' || mData.back() == '?'); }

        friend class object;
    };

    class boolean
    {
     public:
        inline boolean(bool aValue) : mValue{aValue} {}
        inline std::string to_json() const { return mValue ? "true" : "false"; }
        inline std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
        inline operator bool() const { return mValue; }
        inline boolean& operator=(bool aSrc) { mValue = aSrc; return *this; }
        inline void update(const boolean& to_merge) { mValue = to_merge.mValue; }
        inline void remove_comments() {}

     private:
        bool mValue;
    };

    class null
    {
     public:
        inline null() {}
        inline void update(const null&) {}
        inline std::string to_json() const { return "null"; }
        inline std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
        inline void remove_comments() {}
    };

    class number
    {
     public:
        inline number(double aSrc) : mValue{double_to_string(aSrc)} {}
        inline number& operator=(double aSrc) { mValue = double_to_string(aSrc); return *this; }
        inline std::string to_json() const { return mValue; }
        inline std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
        inline operator double() const { return std::stod(mValue); }
        inline void update(const number& to_merge) { mValue = to_merge.mValue; }
        inline void remove_comments() {}

     private:
        inline number(std::string_view&& aData) : mValue{aData} {}

          // double mValue;
        std::string mValue;

        friend class implementation::NumberHandler;
    };

    class integer
    {
     public:
        inline integer(int aSrc) : mValue{std::to_string(aSrc)} {}
        inline integer(unsigned int aSrc) : mValue{std::to_string(aSrc)} {}
        inline integer(long aSrc) : mValue{std::to_string(aSrc)} {}
        inline integer(unsigned long aSrc) : mValue{std::to_string(aSrc)} {}
        inline integer& operator=(int aSrc) { mValue = std::to_string(aSrc); return *this; }
        inline integer& operator=(unsigned int aSrc) { mValue = std::to_string(aSrc); return *this; }
        inline integer& operator=(long aSrc) { mValue = std::to_string(aSrc); return *this; }
        inline integer& operator=(unsigned long aSrc) { mValue = std::to_string(aSrc); return *this; }
        inline std::string to_json() const { return mValue; }
        inline std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
        inline operator double() const { return std::stod(mValue); }
        inline operator long() const { return std::stol(mValue); }
        inline operator unsigned long() const { return std::stoul(mValue); }
        inline operator int() const { return static_cast<int>(std::stol(mValue)); }
        inline operator unsigned int() const { return static_cast<unsigned int>(std::stoul(mValue)); }
        inline void update(const integer& to_merge) { mValue = to_merge.mValue; }
        inline void remove_comments() {}

     private:
        inline integer(std::string_view&& aData) : mValue{aData} {}

        std::string mValue;

        friend class implementation::NumberHandler;
    };

    class object
    {
     public:
        class field_not_found : public std::exception { public: using std::exception::exception; };

        inline object() = default;
        inline object(std::initializer_list<std::pair<string, value>> key_values);

        std::string to_json(bool space_after_comma = false) const;
        std::string to_json_pp(size_t indent, json_pp_emacs_indent emacs_indent = json_pp_emacs_indent::no, size_t prefix = 0) const;

        void insert(const string& aKey, value&& aValue);
        void insert(const string& aKey, const value& aValue);
        void insert(value&& aKey, value&& aValue);
        inline size_t size() const { return mContent.size(); }
        inline bool empty() const { return mContent.empty(); }

          // returns reference to the value at the passed key.
          // if key not found, inserts aDefault with the passed key and returns reference to the inserted
        value& get_ref(std::string aKey, value&& aDefault);
        const value& get_ref(std::string aKey) const; // throws field_not_found
        value& get_ref(const string& aKey); // throws field_not_found
        const value& get_ref(std::string aKey, value&& aDefault) const;
        value get_ref_not_set(std::string aKey, value&& aDefault) const; // if key is absent in the object, returns aDefault but do not update the object
        object& get_ref_to_object(std::string aKey);

        template <typename F> inline std::decay_t<F> get_field(std::string aFieldName, F&& aDefaultValue) const
            {
                return std::get<rjson_type<F>>(get_ref(aFieldName, rjson_type<F>{std::forward<F>(aDefaultValue)}));
            }

        template <typename F> inline std::decay_t<F> get_field(std::string aFieldName) const // throws field_not_found
            {
                return std::get<rjson_type<F>>(get_ref(aFieldName));
            }

        inline const value& operator[](std::string aFieldName) const { return get_ref(aFieldName); } // throws field_not_found

        void set_field(std::string aKey, value&& aValue);
        void set_field(const string& aKey, const value& aValue);
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
    };

    class array
    {
     public:
        inline array() = default;
        inline array(array&&) = default;
        inline array(const array&) = default;
        template <typename ... Args> array(Args ... args);
        inline array& operator=(array&&) = default;
        inline array& operator=(const array&) = default;

        inline void update(const array& to_merge) { mContent = to_merge.mContent; } // replace content!
        void remove_comments();
        std::string to_json(bool space_after_comma = false) const;
        std::string to_json_pp(size_t indent, json_pp_emacs_indent emacs_indent = json_pp_emacs_indent::no, size_t prefix = 0) const;

        void insert(value&& aValue);
        void insert(const value& aValue);
        inline size_t size() const { return mContent.size(); }
        inline bool empty() const { return mContent.empty(); }
        inline value& operator[](size_t index) { return mContent.at(index); }
        inline const value& operator[](size_t index) const { return mContent.at(index); }
        inline void erase(size_t index) { mContent.erase(mContent.begin() + static_cast<std::vector<value>::difference_type>(index)); }
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
    };

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

      // ----------------------------------------------------------------------

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
                    std::cerr << "ERROR: rjson::value::get_ref: not an object: valueless_by_exception:" << valueless_by_exception() << " index:" << index() << " value:" << to_json() << '\n';
                    // std::visit([](auto&& arg) { std::cerr << "actual variant: " << typeid(arg).name() << '\n';}, *this);
                    throw;
                }
            }

        inline object& get_ref_to_object(std::string aKey)
            {
                try {
                    return std::get<object>(*this).get_ref_to_object(aKey);
                }
                catch (std::bad_variant_access&) {
                    std::cerr << "ERROR: rjson::value::get_ref_to_object: not an object: " << to_json() << '\n'; // valueless_by_exception:" << valueless_by_exception() << " index:" << index() << '\n'; // to_json() << '\n';
                    throw;
                }
            }

        template <typename F> inline std::decay_t<F> get_field(std::string aFieldName, F&& aDefaultValue) const
            {
                try {
                    return std::get<object>(*this).get_field(aFieldName, std::forward<F>(aDefaultValue));
                }
                catch (std::bad_variant_access&) {
                    std::cerr << "ERROR: rjson::value::get_field: not an object: " << to_json() << '\n'; // to_json() << '\n';
                    throw;
                }
            }

        template <typename F> inline std::decay_t<F> get_field(std::string aFieldName) const // throws object::field_not_found
            {
                try {
                    return std::get<object>(*this).get_field<F>(aFieldName);
                }
                catch (std::bad_variant_access&) {
                    std::cerr << "ERROR: rjson::value::get_field: not an object: " << to_json() << '\n'; // to_json() << '\n';
                    throw;
                }
            }

        inline double get_field_number(std::string aFieldName) const // throws object::field_not_found
            {
                try {
                    return std::get<object>(*this).get_ref(aFieldName);
                }
                catch (std::bad_variant_access&) {
                    std::cerr << "ERROR: rjson::value::get_field: not an object: " << to_json() << '\n'; // to_json() << '\n';
                    throw;
                }
            }

        template <typename F> inline void set_field(std::string aFieldName, F&& aValue)
            {
                set_field(aFieldName, to_value(aValue));
            }

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

        inline operator const object&() const { return std::get<object>(*this); }
        inline operator const array&() const { return std::get<array>(*this); }

          // ----------------------------------------------------------------------

        value& update(const value& to_merge);
        void remove_comments(); // defined below as inline (gcc 7.2 cannot handle it inlined here)

        std::string to_json() const;
        std::string to_json_pp(size_t indent = 2, json_pp_emacs_indent emacs_indent = json_pp_emacs_indent::yes, size_t prefix = 0) const;

    }; // class value

      // ----------------------------------------------------------------------

    template <typename FValue> inline value to_value(const FValue& aValue)
    {
        return rjson_type<FValue>{aValue};
    }

      // ----------------------------------------------------------------------

    template <typename Result> class value_visitor_base
    {
     public:
        class unexpected_value : public std::runtime_error { public: using std::runtime_error::runtime_error; };

        inline value_visitor_base() = default;
          //inline value_visitor_base(const value_visitor_base&) = default;
          // inline value_visitor_base(value_visitor_base&&) = default;
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
            : mMessage{std::to_string(aLine) + ":" + std::to_string(aColumn) + ": " + std::move(aMessage)} //, mLine{aLine}, mColumn{aColumn}
            {}

        inline const char* what() const noexcept override { return mMessage.c_str(); }

     private:
        std::string mMessage;
          //size_t mLine, mColumn;

    }; // class parse_error

    class merge_error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    value parse_string(std::string aJsonData, bool aRemoveComments = true);
    value parse_string(const char* aJsonData, bool aRemoveComments = true);
    value parse_file(std::string aFilename, bool aRemoveComments = true);

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
    inline void object::insert(value&& aKey, value&& aValue) { insert(std::get<string>(std::forward<value>(aKey)), std::forward<value>(aValue)); }

    inline object::object(std::initializer_list<std::pair<string, value>> key_values)
    {
        for (const auto& [key, value]: key_values)
            insert(key, value);
    }

    inline value& object::get_ref(std::string aKey, value&& aDefault)
    {
        return mContent.emplace(aKey, std::forward<value>(aDefault)).first->second;
    }

    inline const value& object::get_ref(std::string aKey, value&& aDefault) const { return const_cast<object*>(this)->get_ref(aKey, std::forward<value>(aDefault)); }

    inline const value& object::get_ref(std::string aKey) const
    {
        const auto existing = mContent.find(aKey);
        if (existing == mContent.end())
            throw field_not_found{};
        return existing->second;
    }

      // if key is absent in the object, returns aDefault but do not update the object
    inline value object::get_ref_not_set(std::string aKey, value&& aDefault) const
    {
        const auto existing = mContent.find(aKey);
        if (existing == mContent.end())
            return aDefault;
        else
            return existing->second;
    }

    inline value& object::get_ref(const string& aKey)
    {
        const auto existing = mContent.find(aKey);
        if (existing == mContent.end())
            throw field_not_found{};
        return existing->second;
    }

    inline object& object::get_ref_to_object(std::string aKey)
    {
        const auto existing = mContent.find(aKey);
        if (existing == mContent.end())
            return std::get<object>(get_ref(aKey, object{}));
        else
            return std::get<object>(existing->second);
    }

    inline void object::set_field(std::string aKey, value&& aValue)
    {
        mContent.insert_or_assign(aKey, std::forward<value>(aValue));
    }

    inline void object::set_field(const string& aKey, const value& aValue)
    {
        mContent.insert_or_assign(aKey, aValue);
    }

    inline void object::delete_field(string aKey)
    {
        if (auto iter = mContent.find(aKey); iter != mContent.end())
            mContent.erase(iter);
        else
            throw field_not_found{};
    }

      // ----------------------------------------------------------------------

    inline void array::insert(value&& aValue) { mContent.push_back(std::move(aValue)); }
    inline void array::insert(const value& aValue) { mContent.push_back(aValue); }

    template <typename ... Args> inline array::array(Args ... args)
    {
        (insert(to_value(args)), ...);
    }

    // template <typename T> inline array::array(std::initializer_list<T> args)
    // {
    //       // std::for_each(std::begin(args), std::end(args), [this](const T& arg) { this->insert(to_value(arg)); });
    //     // for (auto&& arg: args)
    //     //     insert(to_value(std::forward<T>(arg)));
    //     const T& tt = *args.begin();
    //     using TT = typename content_type<T>::type;
    //     insert(TT(tt));
    // }

      // ----------------------------------------------------------------------

    template <> inline void value::set_field(std::string aFieldName, value&& aValue)
    {
        try {
            std::get<object>(*this).set_field(aFieldName, std::forward<value>(aValue));
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

inline std::ostream& operator<<(std::ostream& out, const rjson::value& aValue)
{
    return out << aValue.to_json();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
