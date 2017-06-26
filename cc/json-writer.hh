#pragma once

#include <string>
#include <vector>
#include <map>

#include "acmacs-base/config.hh"

#pragma GCC diagnostic push
#include "acmacs-base/rapidjson-diagnostics.hh"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#pragma GCC diagnostic pop

#ifdef ACMACSD_FILESYSTEM
#include "acmacs-base/read-file.hh"
#endif
#include "acmacs-base/float.hh"

// ----------------------------------------------------------------------
// Forward declarations
// ----------------------------------------------------------------------

namespace json_writer
{
    template <typename RW> class writer;
    class key;
}

template <typename RW, typename T> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, const std::vector<T>&);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, const char*);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, std::string);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, double);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, int);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, size_t);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, bool);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, const std::map<std::string, std::vector<std::string>>&);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, json_writer::key);

// ======================================================================

namespace json_writer
{
    template <typename RW> class writer : public RW // JsonWriterT
    {
     public:
        inline writer(std::string aKeyword) : RW(mBuffer), mKeyword(aKeyword) {}
        inline operator std::string() const { return mBuffer.GetString(); }
        inline std::string keyword() const { return mKeyword; }

     private:
        rapidjson::StringBuffer mBuffer;
        std::string mKeyword;
    };

      // template <> inline writer<rapidjson::PrettyWriter<rapidjson::StringBuffer>>::writer(std::string aKeyword)
      //     : rapidjson::PrettyWriter<rapidjson::StringBuffer>(mBuffer), mKeyword(aKeyword)
      // {
      //     SetIndent(' ', 1);
      // }

      // ----------------------------------------------------------------------

    enum _StartArray { start_array };
    enum _EndArray { end_array };
    enum _StartObject { start_object };
    enum _EndObject { end_object };

    class key
    {
     public:
        inline key(const char* v) : value(v) {}
        inline key(std::string v) : value(v) {}
        inline key(char v) : value(1, v) {}
        inline operator const char*() const { return value.c_str(); }
        inline const char* as_char_ptr() const { return value.c_str(); }

     private:
        std::string value;
    };

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------

    namespace _internal
    {
          // void_t is a C++17 feature
        template<class ...> using void_t = void; // http://stackoverflow.com/questions/26513095/void-t-can-implement-concepts
        template <typename T, typename = void> struct castable_to_char : public std::false_type {};
        template <typename T> struct castable_to_char<T, void_t<decltype(static_cast<char>(std::declval<T>()))>> : public std::true_type {};
    }

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

    template <typename Key, typename Value> class _if_not_empty
    {
     public:
        inline _if_not_empty(Key&& aKey, Value&& aValue) : mKey(aKey), mValue(aValue) {}

        template <typename RW> friend inline writer<RW>& operator <<(writer<RW>& aWriter, const _if_not_empty<Key, Value>& data)
            {
                if (!data.mValue.empty())
                    aWriter << data.mKey << data.mValue;
                return aWriter;
            }

     private:
        Key mKey;
        Value mValue;
    };

    template <typename Key, typename Value> inline auto if_not_empty(Key&& aKey, Value&& aValue) { return _if_not_empty<Key, Value>(std::forward<Key>(aKey), std::forward<Value>(aValue)); }
    template <typename Value> inline auto if_not_empty(const char* aKey, Value&& aValue) { return _if_not_empty<key, Value>(key(aKey), std::forward<Value>(aValue)); }

// ----------------------------------------------------------------------

    template <typename Key, typename Value> class _if_non_negative
    {
     public:
        inline _if_non_negative(Key&& aKey, Value&& aValue) : mKey(aKey), mValue(aValue) {}

        template <typename RW> friend inline writer<RW>& operator <<(writer<RW>& aWriter, const _if_non_negative<Key, Value>& data)
            {
                if (data.mValue >= 0.0)
                    aWriter << data.mKey << data.mValue;
                return aWriter;
            }

     private:
        Key mKey;
        Value mValue;
    };

    template <typename Key, typename Value> inline auto if_non_negative(Key&& aKey, Value&& aValue) { return _if_non_negative<Key, Value>(std::forward<Key>(aKey), std::forward<Value>(aValue)); }
    template <typename Value> inline auto if_non_negative(const char* aKey, Value&& aValue) { return _if_non_negative<key, Value>(key(aKey), std::forward<Value>(aValue)); }

// ----------------------------------------------------------------------

    template <typename Key, typename Value> class _if_not
    {
     public:
        inline _if_not(Key&& aKey, Value&& aValue, bool aPred) : mKey(aKey), mValue(aValue), mPred(aPred) {}
        inline _if_not(Key&& aKey, Value aValue, bool aPred) : mKey(aKey), mValue(aValue), mPred(aPred) {}

        template <typename RW> friend inline writer<RW>& operator <<(writer<RW>& aWriter, const _if_not<Key, Value>& data)
            {
                if (!data.mPred)
                    aWriter << data.mKey << data.mValue;
                return aWriter;
            }

     private:
        Key mKey;
        Value mValue;
        bool mPred;
    };

    template <typename Key> inline auto if_not_zero(Key&& aKey, double aValue) { return _if_not<Key, double>(std::forward<Key>(aKey), aValue, float_zero(aValue)); }
    inline auto if_not_zero(const char* aKey, double aValue) { return _if_not<key, double>(key(aKey), aValue, float_zero(aValue)); }

    template <typename Key> inline auto if_not_one(Key&& aKey, double aValue) { return _if_not<Key, double>(std::forward<Key>(aKey), aValue, float_equal(aValue, 1.0)); }
    inline auto if_not_one(const char* aKey, double aValue) { return _if_not<key, double>(key(aKey), aValue, float_equal(aValue, 1.0)); }

    template <typename Key, typename Value> inline auto if_not_equal(Key&& aKey, Value&& aValue, Value aEq) { return _if_not<Key, Value>(std::forward<Key>(aKey), aValue, aValue == aEq); }
    template <typename Value> inline auto if_not_equal(const char* aKey, Value&& aValue, Value aEq) { return _if_not<key, Value>(key(aKey), aValue, aValue == aEq); }

} // namespace json_writer

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

// to allow writer << func(writer, data) where func returns writer
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, json_writer::writer<RW>&) { return aWriter; }

template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, json_writer::_StartArray) { aWriter.StartArray(); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, json_writer::_EndArray) { aWriter.EndArray(); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, json_writer::_StartObject) { aWriter.StartObject(); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, json_writer::_EndObject) { aWriter.EndObject(); return aWriter; }

template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const char* s) { aWriter.String(s, static_cast<unsigned>(strlen(s))); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, std::string s) { aWriter.String(s.c_str(), static_cast<unsigned>(s.size())); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, int value) { aWriter.Int(value); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, size_t value) { aWriter.Uint64(value); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, bool value) { aWriter.Bool(value); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, double value) { aWriter.Double(value); return aWriter; }

template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, json_writer::key value) { aWriter.Key(value.as_char_ptr()); return aWriter; }

template <typename RW, typename T> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const std::vector<T>& aList)
{
    aWriter << json_writer::start_array;
    for (const auto& e: aList)
        aWriter << e;
    return aWriter << json_writer::end_array;
}

template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const std::vector<std::vector<std::string>>& list_list_strings)
{
    aWriter << json_writer::start_array;
    for (const auto& e: list_list_strings)
        aWriter << e;
    return aWriter << json_writer::end_array;
}

template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const std::map<std::string, std::vector<std::string>>& map_list_strings)
{
    aWriter << json_writer::start_object;
    for (const auto& e: map_list_strings)
        aWriter << json_writer::key(e.first) << e.second;
    return aWriter << json_writer::end_object;
}

template <typename RW, typename Key, typename std::enable_if<json_writer::_internal::castable_to_char<Key>{}>::type* = nullptr>
                      inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, Key aKey)
{
    const char k = static_cast<char>(aKey);
    aWriter.Key(&k, 1, false);
    return aWriter;
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

namespace json_writer
{
    template <typename V> inline std::string pretty_json(const V& value, std::string keyword, size_t indent, bool insert_emacs_indent_hint)
    {
        writer<rapidjson::PrettyWriter<rapidjson::StringBuffer>> aWriter(keyword);
        aWriter.SetIndent(' ', static_cast<unsigned int>(indent));
        aWriter << value;
        std::string result = aWriter;
        if (insert_emacs_indent_hint && result[0] == '{') {
            const std::string ind(indent - 1, ' ');
            result.insert(1, ind + "\"_\": \"-*- js-indent-level: " + std::to_string(indent) + " -*-\",");
        }
        return result;
    }

    template <typename V> inline std::string compact_json(const V& aValue, std::string keyword)
    {
        writer<rapidjson::Writer<rapidjson::StringBuffer>> aWriter(keyword);
        return aWriter << aValue;
    }

    template <typename V> inline std::string json(const V& value, std::string keyword, size_t indent, bool insert_emacs_indent_hint = true)
    {
        if (indent)
            return pretty_json(value, keyword, indent, insert_emacs_indent_hint);
        else
            return compact_json(value, keyword);
    }

      // ----------------------------------------------------------------------

#ifdef ACMACSD_FILESYSTEM
    template <typename V> inline void export_to_json(const V& value, std::string keyword, std::string filename, size_t indent, bool insert_emacs_indent_hint = true, bool force_compression = false)
    {
        acmacs_base::write_file(filename, json(value, keyword, indent, insert_emacs_indent_hint), force_compression);
    }
#endif

} // namespace json_writer

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
