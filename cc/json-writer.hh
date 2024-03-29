#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

#include "acmacs-base/read-file.hh"
#include "acmacs-base/float.hh"
#include "acmacs-base/sfinae.hh"
#include "acmacs-base/rapidjson.hh"

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
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, std::string_view);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, double);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, int);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, size_t);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, bool);
template <typename RW, typename Value> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const std::vector<Value>& aList);
template <typename RW, typename Value> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const std::set<Value>& aList);
template <typename RW, typename Value> json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const std::map<std::string, Value>& map_value);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, json_writer::key);

// ======================================================================

namespace json_writer
{
    template <typename RW> class writer : public RW // JsonWriterT
    {
     public:
        inline writer() : RW(mBuffer) {}
        inline std::string to_string() const { return mBuffer.GetString(); }

     private:
        rapidjson::StringBuffer mBuffer;
    };

      // ----------------------------------------------------------------------

    enum StartArray { start_array };
    enum EndArray { end_array };
    enum StartObject { start_object };
    enum EndObject { end_object };

    class key
    {
     public:
        inline key(const char* v) : value(v) {}
        inline key(std::string_view v) : value{v} {}
        inline key(char v) : value(1, v) {}
        inline operator const char*() const { return value.c_str(); }
        inline const char* as_char_ptr() const { return value.c_str(); }

     private:
        std::string value;
    };
}

// ----------------------------------------------------------------------

template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, json_writer::StartArray) { aWriter.StartArray(); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, json_writer::EndArray) { aWriter.EndArray(); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, json_writer::StartObject) { aWriter.StartObject(); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, json_writer::EndObject) { aWriter.EndObject(); return aWriter; }

template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, json_writer::key value) { aWriter.Key(value.as_char_ptr()); return aWriter; }

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

namespace json_writer
{
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

    template <typename RW, typename List> inline writer<RW>& write_list(writer<RW>& aWriter, const List& aList)
    {
          // start_array and end_array do not work here due to namespace issue (in clang only?)
        aWriter.StartArray();
        for (const auto& e: aList)
            aWriter << e;
        aWriter.EndArray();
        return aWriter;
    }

    template <typename RW, typename List> inline writer<RW>& write_list(writer<RW>& aWriter, List&& aList)
    {
          // start_array and end_array do not work here due to namespace issue (in clang only?)
        aWriter.StartArray();
        for (auto& e: aList)
            aWriter << e;
        aWriter.EndArray();
        return aWriter;
    }

    template <typename RW, typename Object> inline writer<RW>& write_object(writer<RW>& aWriter, const Object& aObject)
    {
          // start_object and end_object do not work here due to namespace issue (in clang only?)
        aWriter.StartObject();
        for (const auto& e: aObject)
            aWriter << json_writer::key(e.first) << e.second;
        aWriter.EndObject();
        return aWriter;
    }

} // namespace json_writer

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

// to allow writer << func(writer, data) where func returns writer
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, json_writer::writer<RW>&) { return aWriter; }

template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const char* s) { aWriter.String(s, static_cast<unsigned>(strlen(s))); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, std::string_view s) { aWriter.String(s.data(), static_cast<unsigned>(s.size())); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, int value) { aWriter.Int(value); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, size_t value) { aWriter.Uint64(value); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, bool value) { aWriter.Bool(value); return aWriter; }
template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, double value) { aWriter.Double(value); return aWriter; }

template <typename RW, typename Value> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const std::vector<Value>& aList)
{
    return json_writer::write_list(aWriter, aList);
}

template <typename RW, typename Value> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const std::set<Value>& aList)
{
    return json_writer::write_list(aWriter, aList);
}

template <typename RW, typename Value> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const std::map<std::string, Value>& map_value)
{
    return json_writer::write_object(aWriter, map_value);
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

namespace json_writer
{
    enum Finalize { finalize };

    class pretty;
}

std::string operator << (json_writer::pretty& aWriter, json_writer::Finalize);

namespace json_writer
{
    inline void insert_emacs_indent_hint(std::string& aJson, size_t aIndent)
    {
        if (aJson.size() > 2 && aJson[0] == '{') {
            const std::string ind(aIndent - 1, ' ');
            aJson.insert(1, ind + "\"_\": \"-*- js-indent-level: " + std::to_string(aIndent) + " -*-\",");
        }
    }

    using compact = writer<rapidjson::Writer<rapidjson::StringBuffer>>;

    class pretty : public writer<rapidjson::PrettyWriter<rapidjson::StringBuffer>>
    {
     public:
        inline pretty(size_t indent = 1) : writer<rapidjson::PrettyWriter<rapidjson::StringBuffer>>{}, mIndent{indent}
            {
                SetIndent(' ', static_cast<unsigned int>(indent));
            }

        inline size_t indent() const { return mIndent; }
        inline operator std::string() { return *this << finalize; }

     private:
        size_t mIndent;

    }; // class pretty

} // namespace json_writer

// ----------------------------------------------------------------------

inline std::string operator << (json_writer::compact& aWriter, json_writer::Finalize)
{
    return aWriter.to_string();
}

inline std::string operator << (json_writer::pretty& aWriter, json_writer::Finalize)
{
    std::string result = aWriter.to_string();
    json_writer::insert_emacs_indent_hint(result, aWriter.indent());
    return result;
}

// ----------------------------------------------------------------------

namespace json_writer
{
    template <typename V> inline std::string pretty_json(const V& value, size_t indent)
    {
        pretty aWriter{indent};
        aWriter << value;
        return aWriter << finalize;
    }

    template <typename V> inline std::string compact_json(const V& aValue)
    {
        compact aWriter;
        aWriter << aValue;
        return aWriter << finalize;
    }

    template <typename V> inline std::string json(const V& value, size_t indent)
    {
        if (indent)
            return pretty_json(value, indent);
        else
            return compact_json(value);
    }

      // ----------------------------------------------------------------------

    using acmacs::file::force_compression;

    template <typename V> inline void export_to_json(const V& value, std::string_view filename, size_t indent, force_compression a_force_compression = force_compression::no)
    {
        acmacs::file::write(filename, json(value, indent), a_force_compression);
    }

} // namespace json_writer

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
