#pragma once

#include <string>
#include <vector>
#include <map>

#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"

#include "acmacs-base/read-file.hh"

// ----------------------------------------------------------------------

template <typename RW> class JsonWriterT : public RW
{
 public:
    inline JsonWriterT(std::string aKeyword) : RW(mBuffer), mKeyword(aKeyword) {}
    inline operator std::string() const { return mBuffer.GetString(); }
    inline std::string keyword() const { return mKeyword; }

 private:
    rapidjson::StringBuffer mBuffer;
    std::string mKeyword;
};

// template <> inline JsonWriterT<rapidjson::PrettyWriter<rapidjson::StringBuffer>>::JsonWriterT(std::string aKeyword)
//     : rapidjson::PrettyWriter<rapidjson::StringBuffer>(mBuffer), mKeyword(aKeyword)
// {
//     SetIndent(' ', 1);
// }

typedef JsonWriterT<rapidjson::Writer<rapidjson::StringBuffer>> JsonWriter;
typedef JsonWriterT<rapidjson::PrettyWriter<rapidjson::StringBuffer>> JsonPrettyWriter;

// ----------------------------------------------------------------------

enum _StartArray { StartArray };
enum _EndArray { EndArray };
enum _StartObject { StartObject };
enum _EndObject { EndObject };

template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, _StartArray) { writer.StartArray(); return writer; }
template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, _EndArray) { writer.EndArray(); return writer; }
template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, _StartObject) { writer.StartObject(); return writer; }
template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, _EndObject) { writer.EndObject(); return writer; }

template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, const char* s) { writer.String(s, static_cast<unsigned>(strlen(s))); return writer; }
template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, std::string s) { writer.String(s.c_str(), static_cast<unsigned>(s.size())); return writer; }
template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, int value) { writer.Int(value); return writer; }
template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, double value) { writer.Double(value); return writer; }

class JsonObjectKey
{
 public:
    inline JsonObjectKey(const char* v) : value(v) {}
    inline JsonObjectKey(std::string v) : value(v.c_str()) {}
    inline operator const char*() const { return value; }

 private:
    const char* value;
};

template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, JsonObjectKey value) { writer.Key(value); return writer; }

// ----------------------------------------------------------------------

template <typename RW, typename T> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, const std::vector<T>& list)
{
    writer << StartArray;
    for (const auto& e: list)
        writer << e;
    return writer << EndArray;
}

// ----------------------------------------------------------------------

template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, const std::vector<std::vector<std::string>>& list_list_strings)
{
    writer << StartArray;
    for (const auto& e: list_list_strings)
        writer << e;
    return writer << EndArray;
}

// ----------------------------------------------------------------------

template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, const std::map<std::string, std::vector<std::string>>& map_list_strings)
{
    writer << StartObject;
    for (const auto& e: map_list_strings)
        writer << JsonObjectKey(e.first) << e.second;
    return writer << EndObject;
}

// ----------------------------------------------------------------------

template <typename Key, typename Value> class _if_not_empty
{
 public:
    inline _if_not_empty(Key&& key, Value&& value) : mKey(key), mValue(value) {}

    template <typename RW> friend inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, const _if_not_empty<Key, Value>& data)
        {
            if (!data.mValue.empty())
                writer << data.mKey << data.mValue;
            return writer;
        }

 private:
    Key mKey;
    Value mValue;
};

template <typename Key, typename Value> inline auto if_not_empty(Key&& key, Value&& value) { return _if_not_empty<Key, Value>(std::forward<Key>(key), std::forward<Value>(value)); }

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

template <typename V> inline std::string pretty_json(const V& value, std::string keyword, size_t indent, bool insert_emacs_indent_hint)
{
    JsonPrettyWriter writer(keyword);
    writer.SetIndent(' ', static_cast<unsigned int>(indent));
    writer << value;
    std::string result = writer;
    if (insert_emacs_indent_hint && result[0] == '{') {
        const std::string ind(indent - 1, ' ');
        result.insert(1, ind + "\"_\": \"-*- js-indent-level: " + std::to_string(indent) + " -*-\",");
    }
    return result;
}

template <typename V> inline std::string compact_json(const V& value, std::string keyword)
{
    JsonWriter writer(keyword);
    return writer << value;
}

template <typename V> inline std::string json(const V& value, std::string keyword, size_t indent, bool insert_emacs_indent_hint = true)
{
    if (indent)
        return pretty_json(value, keyword, indent, insert_emacs_indent_hint);
    else
        return compact_json(value, keyword);
}

// ----------------------------------------------------------------------

template <typename V> inline void export_to_json(const V& value, std::string keyword, std::string filename, size_t indent, bool insert_emacs_indent_hint = true)
{
    acmacs_base::write_file(filename, json(value, keyword, indent, insert_emacs_indent_hint));
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
