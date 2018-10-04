#pragma once

#include <stdexcept>
#include <variant>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <iostream>
#include <cassert>
#include <typeinfo>
#include <optional>

#include "acmacs-base/debug.hh"
#include "acmacs-base/float.hh"
#include "acmacs-base/to-string.hh"

// ----------------------------------------------------------------------

namespace rjson
{
    namespace v1
    {
        class field_not_found : public std::runtime_error
        {
          public:
            field_not_found(std::string aFieldName = std::string{"field_not_found"}) : std::runtime_error{aFieldName} {}
        };
        class field_type_mismatch : public std::runtime_error
        {
          public:
            field_type_mismatch(std::string aFieldName = std::string{"field_type_mismatch"}) : std::runtime_error{aFieldName} {}
        };
        class numeric_value_is_nan : public std::runtime_error
        {
          public:
            numeric_value_is_nan(std::string msg = std::string{"field is NaN"}) : std::runtime_error{msg} {}
        };

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
            string() = default;
            string(std::string aData) : mData(std::move(aData)) {}
            string(const char* aData) : mData(aData) {}
            string(const char* aData, size_t aLen) : mData(aData, aLen) {}
            string(const string&) = default;
            string& operator=(const string&) = default;
            string(string&&) = default;
            string& operator=(string&&) = default;
            std::string to_json() const { return std::string{"\""} + static_cast<std::string>(mData) + "\""; }
            std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
            // explicit operator std::string() const { return mData; }
            // const std::string& str() const { return mData; }
            operator std::string_view() const { return mData; }
            std::string_view strv() const { return mData; }
            std::string str() const { return mData; }
            string& operator=(std::string aSrc)
            {
                mData = aSrc;
                return *this;
            }
            bool operator==(const std::string& aToCompare) const { return mData == aToCompare; }
            bool operator!=(const std::string& aToCompare) const { return !operator==(aToCompare); }
            bool operator==(std::string_view aToCompare) const { return mData == aToCompare; }
            bool operator!=(std::string_view aToCompare) const { return !operator==(aToCompare); }
            bool operator==(const string& aToCompare) const { return mData == aToCompare.mData; }
            bool operator!=(const string& aToCompare) const { return !operator==(aToCompare); }
            bool operator==(const char* aToCompare) const { return mData == aToCompare; }
            bool operator!=(const char* aToCompare) const { return !operator==(aToCompare); }
            size_t size() const { return mData.size(); }
            bool empty() const { return mData.empty(); }
            char front() const { return mData.front(); }
            char back() const { return mData.back(); }
            bool operator<(const string& to_compare) const { return mData < to_compare.mData; }
            void update(const string& to_merge) { mData = to_merge.mData; }
            constexpr void remove_comments() {}
            template <typename Index>[[noreturn]] const value& operator[](Index) const { throw field_not_found(); }
            template <typename Index>[[noreturn]] value& operator[](Index) { throw field_not_found(); }
            template <typename T>
            [[noreturn]] value& get_or_add(std::string, T&&) { throw field_not_found(); }

            private : std::string mData;

            bool is_comment_key() const { return !mData.empty() && (mData.front() == '?' || mData.back() == '?'); }

            friend class object;

        }; // class string

        inline std::string operator+(std::string left, const string& right) { return left + right.str(); }

        class boolean
        {
          public:
            boolean() : mValue{false} {}
            boolean(bool aValue) : mValue{aValue} {}
            std::string to_json() const { return mValue ? "true" : "false"; }
            std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
            constexpr operator bool() const { return mValue; }
            boolean& operator=(bool aSrc)
            {
                mValue = aSrc;
                return *this;
            }
            void update(const boolean& to_merge) { mValue = to_merge.mValue; }
            constexpr void remove_comments() {}
            template <typename Index>[[noreturn]] const value& operator[](Index) const { throw field_not_found(); }
            template <typename Index>[[noreturn]] value& operator[](Index) { throw field_not_found(); }
            template <typename T>
            [[noreturn]] value& get_or_add(std::string, T&&) { throw field_not_found(); }

            private : bool mValue;

        }; // class boolean

        class null
        {
          public:
            null() {}
            void update(const null&) {}
            std::string to_json() const { return "null"; }
            std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
            constexpr void remove_comments() {}
            bool operator==(null) const { return true; }
            template <typename Index>[[noreturn]] const value& operator[](Index) const { throw field_not_found(); }
            template <typename Index>[[noreturn]] value& operator[](Index) { throw field_not_found(); }
            template <typename T>[[noreturn]] value& get_or_add(std::string, T&&) { throw field_not_found(); }

        }; // class null

        class number
        {
          private:
            template <typename D> inline static std::string make_value(D val, int precision = 32)
            {
                if (std::isnan(val))
                    throw numeric_value_is_nan{};
                return acmacs::to_string(val, precision);
            }

          public:
            number() : mValue{"0.0"} {}
            number(double aSrc) : mValue{make_value(aSrc)} {}
            number(long double aSrc) : mValue{make_value(aSrc)} {}
            number(double aSrc, int precision) : mValue{make_value(aSrc, precision)} {}
            number(std::string_view aData) : mValue{aData} {} // for parser
            number& operator=(double aSrc)
            {
                mValue = make_value(aSrc);
                return *this;
            }
            std::string to_json() const { return mValue; }
            std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
            operator double() const { return std::stod(mValue); }
            void update(const number& to_merge) { mValue = to_merge.mValue; }
            constexpr void remove_comments() {}
            template <typename Index>[[noreturn]] const value& operator[](Index) const { throw field_not_found(); }
            template <typename Index>[[noreturn]] value& operator[](Index) { throw field_not_found(); }
            template <typename T>
            [[noreturn]] value& get_or_add(std::string, T&&) { throw field_not_found(); }

            private :
                // double mValue;
                std::string mValue;

        }; // class number

        class integer
        {
          public:
            integer() : mValue{"0"} {}
            integer(int aSrc) : mValue{acmacs::to_string(aSrc)} {}
            integer(unsigned int aSrc) : mValue{acmacs::to_string(aSrc)} {}
            integer(long aSrc) : mValue{acmacs::to_string(aSrc)} {}
            integer(unsigned long aSrc) : mValue{acmacs::to_string(aSrc)} {}
            integer(unsigned long long aSrc) : mValue{acmacs::to_string(aSrc)} {}
            integer(std::string_view aData) : mValue{aData} {} // for parser
            integer& operator=(int aSrc)
            {
                mValue = acmacs::to_string(aSrc);
                return *this;
            }
            integer& operator=(unsigned int aSrc)
            {
                mValue = acmacs::to_string(aSrc);
                return *this;
            }
            integer& operator=(long aSrc)
            {
                mValue = acmacs::to_string(aSrc);
                return *this;
            }
            integer& operator=(unsigned long aSrc)
            {
                mValue = acmacs::to_string(aSrc);
                return *this;
            }
            std::string to_json() const { return mValue; }
            std::string to_json_pp(size_t, json_pp_emacs_indent = json_pp_emacs_indent::no, size_t = 0) const { return to_json(); }
            operator double() const { return std::stod(mValue); }
            operator long() const { return std::stol(mValue); }
            operator unsigned long() const { return std::stoul(mValue); }
            operator int() const { return static_cast<int>(std::stol(mValue)); }
            operator unsigned int() const { return static_cast<unsigned int>(std::stoul(mValue)); }
            operator bool() const { return static_cast<int>(*this); } // using integer as bool
            void update(const integer& to_merge) { mValue = to_merge.mValue; }
            constexpr void remove_comments() {}
            template <typename Index>[[noreturn]] const value& operator[](Index) const { throw field_not_found(); }
            template <typename Index>[[noreturn]] value& operator[](Index) { throw field_not_found(); }
            template <typename T>
            [[noreturn]] value& get_or_add(std::string, T&&) { throw field_not_found(); }

            private : std::string mValue;

        }; // class integer

        class object
        {
          public:
            object() = default;
            object(std::initializer_list<std::pair<string, value>> key_values);
            // ~object() { std::cerr << "~object " << to_json(true) << '\n'; }
            object(const object&) = default;            // required if explicit destructor provided
            object& operator=(const object&) = default; // required if explicit destructor provided
            object(object&&) = default;
            object& operator=(object&&) = default;

            std::string to_json(bool space_after_comma = false) const;
            std::string to_json_pp(size_t indent, json_pp_emacs_indent emacs_indent = json_pp_emacs_indent::no, size_t prefix = 0) const;

            void insert(const string& aKey, value&& aValue);
            void insert(const string& aKey, const value& aValue);
            void insert(value&& aKey, value&& aValue);
            size_t size() const { return mContent.size(); }
            bool empty() const { return mContent.empty(); }

            // if field is not in the object, throws field_not_found
            const value& operator[](std::string aFieldName) const;
            value& operator[](std::string aFieldName);
            value& operator[](const string& aFieldName) { return operator[](aFieldName.str()); }
            value& operator[](const char* aFieldName) { return operator[](std::string{aFieldName}); }
            const value& one_of(std::initializer_list<std::string> aFieldOrder) const;
            template <typename... Args> const value& one_of(Args... args) const { return one_of({args...}); }
            const value& operator[](const char* aFieldName) const { return operator[](std::string{aFieldName}); }
            template <typename Index>[[noreturn]] const value& operator[](Index) const { throw field_not_found(); }
            template <typename Index>[[noreturn]] value& operator[](Index) { throw field_not_found(); }
            template <typename T> value& get_or_add(std::string aFieldName, T&& aDefault);

            template <typename T> std::optional<T> get(std::string aFieldName) const;
            std::string get_string_or_throw(std::string aFieldName) const;

            template <typename T> std::decay_t<T> get_or_default(std::string aFieldName, T&& aDefault) const;
            std::string get_or_default(std::string aFieldName, const char* aDefault) const { return get_or_default<std::string>(aFieldName, aDefault); }

            template <typename R> std::pair<bool, const R&> get_R_if(std::string aFieldName) const;
            bool exists(std::string aFieldName) const;
            std::pair<bool, const value&> get_value_if(std::string aFieldName) const;
            std::pair<bool, const object&> get_object_if(std::string aFieldName) const;
            std::pair<bool, const array&> get_array_if(std::string aFieldName) const;
            const object& get_or_empty_object(std::string aFieldName) const;
            const array& get_or_empty_array(std::string aFieldName) const;

            value& set_field(std::string aKey, value&& aValue);        // returns ref to inserted
            value& set_field(const string& aKey, const value& aValue); // returns ref to inserted
            template <typename T> void set_field_if_not_empty(std::string aKey, const T& aValue);
            template <typename T> void set_field_if_not_default(std::string aKey, const T& aValue, const T& aDefault);
            void set_field_if_not_default(std::string aKey, double aValue, double aDefault, int precision = 32);
            template <typename Iterator> void set_array_field_if_not_empty(std::string aKey, Iterator first, Iterator last);
            template <typename Container> void set_array_field_if_not_empty(std::string aKey, const Container& aContainer)
            {
                set_array_field_if_not_empty(aKey, std::begin(aContainer), std::end(aContainer));
            }

            void delete_field(string aKey); // throws field_not_found
            void clear() { mContent.clear(); }

            using const_iterator = decltype(std::declval<std::map<string, value>>().cbegin());
            const_iterator begin() const { return mContent.begin(); }
            const_iterator end() const { return mContent.end(); }
            using iterator = decltype(std::declval<std::map<string, value>>().begin());
            iterator begin() { return mContent.begin(); }
            iterator end() { return mContent.end(); }

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
            enum _use_iterator { use_iterator };
            array() = default;
            array(array&&) = default;
            array(const array&) = default;
            template <typename Iterator> array(_use_iterator, Iterator first, Iterator last);
            template <typename... Args> array(Args... args);
            array& operator=(array&&) = default;
            array& operator=(const array&) = default;
            value& operator[](size_t index)
            {
                try {
                    return mContent.at(index);
                }
                catch (std::out_of_range&) {
                    throw field_not_found("No element " + acmacs::to_string(index) + " in rjson::array of size " + acmacs::to_string(size()));
                }
            }
            value& operator[](int index)
            {
                try {
                    return mContent.at(static_cast<decltype(mContent)::size_type>(index));
                }
                catch (std::out_of_range&) {
                    throw field_not_found("No element " + acmacs::to_string(index) + " in rjson::array of size " + acmacs::to_string(size()));
                }
            }
            template <typename Index>[[noreturn]] const value& operator[](Index) const { throw field_not_found(); }
            template <typename Index>[[noreturn]] value& operator[](Index) { throw field_not_found(); }
            template <typename T>
            [[noreturn]] value& get_or_add(std::string, T&&) { throw field_not_found(); }

            void update(const array& to_merge)
            {
                mContent = to_merge.mContent;
            } // replace content!
            void remove_comments();
            std::string to_json(bool space_after_comma = false) const;
            std::string to_json_pp(size_t indent, json_pp_emacs_indent emacs_indent = json_pp_emacs_indent::no, size_t prefix = 0) const;

            value& insert(value&& aValue);      // returns ref to inserted
            value& insert(const value& aValue); // returns ref to inserted
            size_t size() const { return mContent.size(); }
            bool empty() const { return mContent.empty(); }
            const value& operator[](size_t index) const { return mContent.at(index); }
            const value& operator[](int index) const { return mContent.at(static_cast<decltype(mContent)::size_type>(index)); }
            void erase(size_t index);
            void erase(int index);
            void resize(size_t new_size);
            void resize(size_t new_size, const value& to_insert);
            void clear() { mContent.clear(); }

            using iterator = decltype(std::declval<const std::vector<value>>().begin());
            using reverse_iterator = decltype(std::declval<const std::vector<value>>().rbegin());
            iterator begin() const { return mContent.begin(); }
            iterator end() const { return mContent.end(); }
            iterator begin() { return mContent.begin(); }
            iterator end() { return mContent.end(); }
            reverse_iterator rbegin() const { return mContent.rbegin(); }

          private:
            std::vector<value> mContent;

        }; // class array

        // ----------------------------------------------------------------------

        template <> struct content_type<double>
        {
            using type = number;
        };
        template <> struct content_type<long>
        {
            using type = integer;
        };
        template <> struct content_type<unsigned long>
        {
            using type = integer;
        };
        template <> struct content_type<int>
        {
            using type = integer;
        };
        template <> struct content_type<unsigned int>
        {
            using type = integer;
        };
        template <> struct content_type<bool>
        {
            using type = boolean;
        };
        template <> struct content_type<std::string>
        {
            using type = string;
        };
        template <> struct content_type<char*>
        {
            using type = string;
        };
        template <> struct content_type<const char*>
        {
            using type = string;
        };

        template <> struct content_type<null>
        {
            using type = null;
        };
        template <> struct content_type<object>
        {
            using type = object;
        };
        template <> struct content_type<array>
        {
            using type = array;
        };
        template <> struct content_type<string>
        {
            using type = string;
        };
        template <> struct content_type<boolean>
        {
            using type = boolean;
        };
        template <> struct content_type<number>
        {
            using type = number;
        };
        template <> struct content_type<integer>
        {
            using type = integer;
        };

        template <typename FValue> value to_value(const FValue& aValue);
        template <typename FValue> value to_value(FValue&& aValue);

        // ----------------------------------------------------------------------

        using value_base = std::variant<null, object, array, string, integer, number, boolean>; // null must be the first alternative, it is the default value;

        class value : public value_base
        {
          public:
            using value_base::operator=;
            using value_base::value_base;
            value(const value&) = default; // otherwise it is deleted
                                           // value(const value& aSrc) : value_base(aSrc) { std::cerr << "rjson::value copy " << aSrc.to_json() << '\n'; }
            value(value&&) = default;
            // value(value&& aSrc) : value_base(std::move(aSrc)) { std::cerr << "rjson::value move " << to_json() << '\n'; }
            value& operator=(const value&) = default; // otherwise it is deleted
            value& operator=(value&&) = default;
            // ~value() { std::cerr << "DEBUG: ~value " << to_json() << DEBUG_LINE_FUNC << '\n'; }

            // ----------------------------------------------------------------------

            operator unsigned long() const { return std::get<integer>(*this); }
            operator long() const { return std::get<integer>(*this); }
            operator unsigned int() const { return std::get<integer>(*this); }
            operator int() const { return std::get<integer>(*this); }
            // operator std::string() const { return std::get<string>(*this); }
            // const std::string& str() const { return std::get<string>(*this).str(); }
            operator std::string_view() const { return std::get<string>(*this); }
            std::string_view strv() const { return std::get<string>(*this).strv(); }
            std::string str() const { return std::get<string>(*this).str(); }

            operator double() const
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

            operator bool() const
            {
                if (auto ptr_n = std::get_if<boolean>(this))
                    return *ptr_n;
                else if (auto ptr_i = std::get_if<integer>(this))
                    return *ptr_i;
                else {
                    std::cerr << "ERROR: cannot convert json to bool (from rjson::boolean or rjson::integer): " << to_json() << '\n';
                    throw std::bad_variant_access{};
                }
            }

            operator const null&() const { return std::get<null>(*this); }
            operator const boolean&() const { return std::get<boolean>(*this); }
            operator const string&() const { return std::get<string>(*this); }
            operator const integer&() const { return std::get<integer>(*this); }
            operator const number&() const { return std::get<number>(*this); }
            operator const object&() const { return std::get<object>(*this); }
            operator const array&() const { return std::get<array>(*this); }
            operator object&() { return std::get<object>(*this); }
            operator array&() { return std::get<array>(*this); }

            bool empty() const;

            bool operator==(const std::string& aToCompare) const { return std::get<string>(*this) == aToCompare; }
            // bool operator!=(const std::string& aToCompare) const { return ! operator==(aToCompare); }
            bool operator==(std::string_view aToCompare) const { return std::get<string>(*this) == aToCompare; }
            // bool operator!=(std::string_view aToCompare) const { return ! operator==(aToCompare); }
            bool operator==(const string& aToCompare) const { return std::get<string>(*this) == aToCompare; }
            // bool operator!=(const string& aToCompare) const { return ! operator==(aToCompare); }
            bool operator==(const char* aToCompare) const { return std::get<string>(*this) == aToCompare; }
            // bool operator!=(const char* aToCompare) const { return ! operator==(aToCompare); }
            bool operator==(null aToCompare) const
            {
                try {
                    return std::get<null>(*this) == aToCompare;
                }
                catch (...) {
                    return false;
                }
            }

            template <typename T> bool operator!=(T&& to_compare) const { return !operator==(std::forward<T>(to_compare)); }

            // ----------------------------------------------------------------------

            template <typename Index> const value& operator[](Index aIndex) const;

            template <typename Index> value& operator[](Index aIndex)
            {
                return std::visit([&](auto&& arg) -> value& { return arg[aIndex]; }, *this);
            }

            template <typename T> std::optional<T> get(std::string aFieldName) const;
            template <typename T> T get_or_default(std::string aFieldName, T&& aDefault) const;

            std::string get_or_default(std::string aFieldName, const char* aDefault) const { return get_or_default<std::string>(aFieldName, aDefault); }

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

            template <typename R> std::pair<bool, const R&> get_R_if(std::string aFieldName) const
            {
                try {
                    return {true, operator[](aFieldName)};
                }
                catch (std::exception&) {
                    if constexpr (std::is_same_v<R, value>)
                        return {false, sNull};
                    else if constexpr (std::is_same_v<R, object>)
                        return {false, sEmptyObject};
                    else
                        return {false, sEmptyArray};
                }
            }

            std::pair<bool, const value&> get_value_if(std::string aFieldName) const { return get_R_if<value>(aFieldName); }
            std::pair<bool, const object&> get_object_if(std::string aFieldName) const { return get_R_if<object>(aFieldName); }
            std::pair<bool, const array&> get_array_if(std::string aFieldName) const { return get_R_if<array>(aFieldName); }

            template <typename T> value& get_or_add(std::string aFieldName, T&& aDefault)
            {
                return std::visit([&](auto&& arg) -> value& { return arg.get_or_add(aFieldName, std::forward<T>(aDefault)); }, *this);
            }

            template <typename T> value& get_or_add(std::string aFieldName, const T& aDefault)
            {
                T move_default{aDefault};
                return std::visit([&](auto&& arg) -> value& { return arg.get_or_add(aFieldName, std::forward<T>(move_default)); }, *this);
            }

            template <typename F> value& set_field(std::string aFieldName, F&& aValue)
            {
                return std::visit(
                    [&](auto&& arg) -> value& {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, object>)
                            return arg.set_field(aFieldName, std::forward<F>(aValue));
                        else
                            throw field_not_found("No field \"" + aFieldName + "\" in rjson::value");
                    },
                    *this);
            }

            template <typename F> value& set_field(std::string aFieldName, const F& aValue)
            {
                return std::visit(
                    [&](auto&& arg) -> value& {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, object>)
                            return arg.set_field(aFieldName, aValue);
                        else
                            throw field_not_found("No field \"" + aFieldName + "\" in rjson::value");
                    },
                    *this);
            }

            void delete_field(std::string aFieldName);

            value& update(const value& to_merge);
            void remove_comments(); // defined below as inline (gcc 7.2 cannot handle it inlined here)

            std::string to_json() const;
            std::string to_json_pp(size_t indent = 2, json_pp_emacs_indent emacs_indent = json_pp_emacs_indent::yes, size_t prefix = 0) const;

        }; // class value

        // ----------------------------------------------------------------------

        template <> struct content_type<value>
        {
            using type = value;
        };

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

        inline value to_value(double aValue, int precision) { return number(aValue, precision); }

        // template <typename FValue> inline value to_value(value&& aValue) { return aValue; }
        template <typename FValue> inline value to_value(object&& aValue) { return std::move(aValue); }
        template <typename FValue> inline value to_value(array&& aValue) { return std::move(aValue); }

        // template <typename FValue> inline value to_value(const value& aValue) { return aValue; }
        template <typename FValue> inline value to_value(const object& aValue) { return aValue; }
        template <typename FValue> inline value to_value(const array& aValue) { return aValue; }

        // ----------------------------------------------------------------------

        template <typename Result> class value_visitor_base
        {
          public:
            class unexpected_value : public std::runtime_error
            {
              public:
                using std::runtime_error::runtime_error;
            };

            value_visitor_base() = default;
            virtual ~value_visitor_base() {}

            [[noreturn]] virtual Result operator()(null& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
            [[noreturn]] virtual Result operator()(object& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
            [[noreturn]] virtual Result operator()(array& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
            [[noreturn]] virtual Result operator()(string& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
            [[noreturn]] virtual Result operator()(integer& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
            [[noreturn]] virtual Result operator()(number& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
            [[noreturn]] virtual Result operator()(boolean& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }

            [[noreturn]] virtual Result operator()(const null& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
            [[noreturn]] virtual Result operator()(const object& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
            [[noreturn]] virtual Result operator()(const array& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
            [[noreturn]] virtual Result operator()(const string& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
            [[noreturn]] virtual Result operator()(const integer& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
            [[noreturn]] virtual Result operator()(const number& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }
            [[noreturn]] virtual Result operator()(const boolean& aValue) { throw_unexpected_value("unexpected value: " + aValue.to_json()); }

          protected:
            [[noreturn]] virtual void throw_unexpected_value(std::string aMessage) { throw unexpected_value{aMessage}; }

        }; // class value_visitor_base

        // ----------------------------------------------------------------------

        class parse_error : public std::exception
        {
          public:
            parse_error(size_t aLine, size_t aColumn, std::string&& aMessage)
                : mMessage{acmacs::to_string(aLine) + ":" + acmacs::to_string(aColumn) + ": " + std::move(aMessage)} //, mLine{aLine}, mColumn{aColumn}
            {
            }

            const char* what() const noexcept override { return mMessage.c_str(); }

          private:
            std::string mMessage;
            // size_t mLine, mColumn;

        }; // class parse_error

        class merge_error : public std::runtime_error
        {
          public:
            using std::runtime_error::runtime_error;
        };

        enum class remove_comments { No, Yes };

        value parse_string(std::string aJsonData, remove_comments aRemoveComments = remove_comments::Yes);
        value parse_string(std::string_view aJsonData, remove_comments aRemoveComments = remove_comments::Yes);
        value parse_string(const char* aJsonData, remove_comments aRemoveComments = remove_comments::Yes);
        value parse_file(std::string aFilename, remove_comments aRemoveComments = remove_comments::Yes);

    } // namespace v1

} // namespace rjson

// ----------------------------------------------------------------------
// gcc support
// ----------------------------------------------------------------------

#ifndef __clang__
namespace std
{
      // gcc 7.2 wants those, if we derive from std::variant
    template<> struct variant_size<rjson::v1::value> : variant_size<rjson::v1::value_base> {};
    template<size_t _Np> struct variant_alternative<_Np, rjson::v1::value> : variant_alternative<_Np, rjson::v1::value_base> {};
}
#endif

// ----------------------------------------------------------------------
// inline
// ----------------------------------------------------------------------

namespace rjson
{
    namespace v1
    {
        inline void object::insert(const string& aKey, value&& aValue) { mContent.emplace(aKey, std::move(aValue)); }
        inline void object::insert(const string& aKey, const value& aValue) { mContent.emplace(aKey, aValue); }
        inline void object::insert(value&& aKey, value&& aValue) { insert(std::get<string>(std::move(aKey)), std::move(aValue)); }

        inline object::object(std::initializer_list<std::pair<string, value>> key_values)
        {
            for (const auto& [key, value] : key_values)
                insert(key, value);
        }

        inline const value& object::operator[](std::string aFieldName) const
        {
            if (const auto existing = mContent.find(aFieldName); existing != mContent.end())
                return existing->second;
            else
                throw field_not_found("No field \"" + aFieldName + "\" in rjson::object");
        }

        inline value& object::operator[](std::string aFieldName)
        {
            if (const auto existing = mContent.find(aFieldName); existing != mContent.end())
                return existing->second;
            else
                throw field_not_found("No field \"" + aFieldName + "\" in rjson::object");
        }

        inline const value& object::one_of(std::initializer_list<std::string> aFieldOrder) const
        {
            for (const auto& name : aFieldOrder) {
                if (const auto existing = mContent.find(name); existing != mContent.end())
                    return existing->second;
            }
            throw field_not_found("No fields \"" + acmacs::to_string(aFieldOrder.begin(), aFieldOrder.end()) + "\" in rjson::object");
        }

        template <typename T> inline std::optional<T> object::get(std::string aFieldName) const
        {
            static_assert(!std::is_same_v<T, object> && !std::is_same_v<T, array>, "get returns a copy, not a reference, use get_or_empty_object or get_or_empty_array");
            try {
                const auto& val = operator[](aFieldName);
                if constexpr (std::is_same_v<T, std::string>)
                    return std::get<string>(val).str();
                else
                    return static_cast<T>(val);
            }
            catch (field_not_found&) {
                return {};
            }
            catch (std::bad_variant_access&) {
                const value& val = operator[](aFieldName);
                return std::visit(
                    [aFieldName](auto&& arg) -> T {
                        using AT = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<AT, null>)
                            return {};
                        else
                            throw field_type_mismatch("Type mismatch for value of \"" + aFieldName + "\", stored " + typeid(AT).name() + " expected convertible to " + typeid(T).name() +
                                                      " or rjson::null");
                    },
                    val);
            }
        }

        inline std::string object::get_string_or_throw(std::string aFieldName) const { return std::get<string>(operator[](aFieldName)).str(); }

        template <typename T> inline std::decay_t<T> object::get_or_default(std::string aFieldName, T&& aDefault) const
        {
            static_assert(!std::is_same_v<T, object> && !std::is_same_v<T, array>, "get_or_default returns a copy, not a reference, use get_or_empty_object or get_or_empty_array");
            const auto val = get<std::decay_t<T>>(aFieldName);
            return val ? *val : std::forward<T>(aDefault);

            // try {
            //     return operator[](aFieldName);
            // }
            // catch (field_not_found&) {
            //     return std::forward<T>(aDefault);
            // }
            // catch (std::bad_variant_access&) {
            //     const value& val = operator[](aFieldName);
            //     return std::visit([aFieldName,&aDefault](auto&& arg) -> T {
            //         using AT = std::decay_t<decltype(arg)>;
            //         if constexpr (std::is_same_v<AT, rjson::null>)
            //             return std::forward<T>(aDefault);
            //         else
            //             throw field_type_mismatch("Type mismatch for value of \"" + aFieldName + "\", stored " + typeid(AT).name() + " expected convertible to " + typeid(T).name() + " or
            //             rjson::null");
            //     }, val);
            // }
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

        template <> inline value& object::get_or_add(std::string aFieldName, object&& aDefault)
        {
            try {
                return operator[](aFieldName);
            }
            catch (field_not_found&) {
                return set_field(aFieldName, std::move(aDefault));
            }
        }

        template <typename T> inline value& object::get_or_add(std::string aFieldName, T&& aDefault) { return get_or_add(aFieldName, to_value(std::forward<T>(aDefault))); }

        inline value& object::set_field(std::string aKey, value&& aValue) { return mContent.insert_or_assign(aKey, std::forward<value>(aValue)).first->second; }

        inline value& object::set_field(const string& aKey, const value& aValue) { return mContent.insert_or_assign(aKey, aValue).first->second; }

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

        inline void object::set_field_if_not_default(std::string aKey, double aValue, double aDefault, int precision)
        {
            if (!std::isnan(aValue) && !float_equal(aValue, aDefault))
                set_field(aKey, to_value(aValue, precision));
        }

        template <typename Iterator> inline void object::set_array_field_if_not_empty(std::string aKey, Iterator first, Iterator last)
        {
            if (first != last) {
                array& ar = set_field(aKey, array{});
                for (; first != last; ++first)
                    ar.insert(to_value(*first));
            }

        } // object::set_array_field_if_not_empty

        inline void object::delete_field(string aKey)
        {
            using namespace std::string_literals;
            if (auto iter = mContent.find(aKey); iter != mContent.end())
                mContent.erase(iter);
            else
                throw field_not_found("No field \""s + aKey + "\" in rjson::object, cannot delete it");
        }

        inline const object& object::get_or_empty_object(std::string aFieldName) const
        {
            try {
                const auto& v = operator[](aFieldName);
                if (std::get_if<null>(&v))
                    return sEmptyObject;
                return v;
            }
            catch (field_not_found&) {
                return sEmptyObject;
            }
            catch (std::bad_variant_access&) {
                std::cerr << "object::get_or_empty_object " << aFieldName << ": bad_variant_access\n";
                throw;
            }
        }

        inline const array& object::get_or_empty_array(std::string aFieldName) const
        {
            try {
                const auto& v = operator[](aFieldName);
                if (std::get_if<null>(&v))
                    return sEmptyArray;
                return v;
            }
            catch (field_not_found&) {
                return sEmptyArray;
            }
            catch (std::bad_variant_access&) {
                std::cerr << "object::get_or_empty_array " << aFieldName << ": bad_variant_access\n";
                throw;
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
            catch (std::exception&) {
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

        inline value& array::insert(value&& aValue)
        {
            mContent.push_back(std::move(aValue));
            return mContent.back();
        }
        inline value& array::insert(const value& aValue)
        {
            mContent.push_back(aValue);
            return mContent.back();
        }

        template <typename... Args> inline array::array(Args... args) { (insert(to_value(args)), ...); }

        template <typename Iterator> inline array::array(array::_use_iterator, Iterator first, Iterator last)
        {
            for (; first != last; ++first)
                insert(to_value(*first));
        }

    inline void array::erase(size_t index) { mContent.erase(mContent.begin() + static_cast<std::vector<value>::difference_type>(index)); }
           inline void array::erase(int index) { mContent.erase(mContent.begin() + index); }
        inline void array::resize(size_t new_size, const value& to_insert) { mContent.resize(new_size, to_insert); }
        inline void array::resize(size_t new_size) { resize(new_size, null{}); }

        // ----------------------------------------------------------------------

        // gcc 7.2 wants the following functions to be defined here (not inside the class)

        inline bool value::empty() const
        {
            return std::visit(
                [&](auto&& arg) -> bool {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, object> || std::is_same_v<T, array>)
                        return arg.empty();
                    else
                        throw std::bad_variant_access{};
                },
                *this);
        }

        inline void value::delete_field(std::string aFieldName)
        {
            return std::visit(
                [&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, object>)
                        arg.delete_field(aFieldName);
                    else
                        throw field_not_found("No field \"" + aFieldName + "\" in rjson::value, cannot delete it");
                },
                *this);
        }

        template <typename Index> inline const value& value::operator[](Index aIndex) const
        {
            return std::visit([&](auto&& arg) -> const value& { return arg[aIndex]; }, *this);
        }

        template <typename T> inline std::optional<T> value::get(std::string aFieldName) const
        {
            try {
                return static_cast<const object&>(*this).get<T>(aFieldName);
            }
            catch (std::bad_variant_access&) {
                std::cerr << "value::get called for non-object, stored variant alternative: " << index() << '\n';
                throw;
            }
        }

        template <typename T> inline T value::get_or_default(std::string aFieldName, T&& aDefault) const
        {
            try {
                return static_cast<const object&>(*this).get_or_default(aFieldName, std::forward<T>(aDefault));
            }
            catch (std::bad_variant_access&) {
                std::cerr << "value::get_or_default called for non-object, stored variant alternative: " << index() << '\n';
                throw;
            }
        }

        inline const object& value::get_or_empty_object(std::string aFieldName) const
        {
            try {
                return operator[](aFieldName);
            }
            catch (field_not_found&) {
                return sEmptyObject;
            }
            catch (std::bad_variant_access&) {
                const value& val = operator[](aFieldName);
                return std::visit(
                    [aFieldName](auto&& arg) -> const object& {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, null>)
                            return sEmptyObject;
                        else
                            throw field_type_mismatch("Type mismatch for value of \"" + aFieldName + "\", stored " + typeid(T).name() + " expected rjson::object or rjson::null");
                    },
                    val);
            }
        }

        inline const array& value::get_or_empty_array(std::string aFieldName) const
        {
            try {
                return operator[](aFieldName);
            }
            catch (field_not_found&) {
                return sEmptyArray;
            }
            catch (std::bad_variant_access&) {
                const value& val = operator[](aFieldName);
                return std::visit(
                    [aFieldName](auto&& arg) -> const array& {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, null>)
                            return sEmptyArray;
                        else
                            throw field_type_mismatch("Type mismatch for value of \"" + aFieldName + "\", stored " + typeid(T).name() + " expected rjson::array or rjson::null");
                    },
                    val);
            }
        }

        template <> inline value& value::set_field(std::string aFieldName, value&& aValue)
        {
            try {
                return std::get<object>(*this).set_field(aFieldName, std::move(aValue));
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

        namespace literals
        {
            inline string operator"" _rj(const char* str, std::size_t len) { return string(str, len); }
            inline string operator"" _rj(const char* str) { return string(str); }
            inline integer operator"" _rj(unsigned long long i) { return i; }
            inline number operator"" _rj(long double d) { return d; }

        } // namespace literals

    }     // namespace v1
} // namespace rjson

// ----------------------------------------------------------------------

namespace acmacs::sfinae
{
    template <typename T, typename = void> struct has_to_json : std::false_type { };
    template <typename T> struct has_to_json<T, std::void_t<decltype(std::declval<const T>().to_json())>> : std::true_type { };
}

template <typename T> inline typename std::enable_if_t<acmacs::sfinae::has_to_json<T>::value, std::ostream&> operator<<(std::ostream& out, const T& aValue)
{
    return out << aValue.to_json();
}

// ----------------------------------------------------------------------

namespace acmacs
{
    inline std::string to_string(const rjson::v1::string& src) { return src.str(); }
    inline std::string to_string(const rjson::v1::null&) { return "null"; }
    inline std::string to_string(const rjson::v1::object& src) { return src.to_json(); }
    inline std::string to_string(const rjson::v1::array& src) { return src.to_json(); }
    inline std::string to_string(const rjson::v1::integer& src) { return src.to_json(); }
    inline std::string to_string(const rjson::v1::number& src) { return src.to_json(); }
    inline std::string to_string(const rjson::v1::boolean& src) { return src.to_json(); }

    inline std::string to_string(const rjson::v1::value& src)
    {
        return std::visit([](auto&& arg) -> std::string { return acmacs::to_string(arg); }, src);
    }

}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
