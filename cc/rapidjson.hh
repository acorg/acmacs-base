#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>

// ----------------------------------------------------------------------

class RapidjsonAssert : public std::exception { public: using std::exception::exception; };

#define RAPIDJSON_ASSERT(x) {if (!(x)) throw RapidjsonAssert{}; }

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#include "acmacs-base/rapidjson-diagnostics.hh"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

namespace json_importer
{
    template <typename Value> Value get(const rapidjson::Value& aValue, const char* aName);

    template<> inline std::string get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetString(); }
    template<> inline size_t get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetUint(); }
    template<> inline unsigned get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetUint(); }
    template<> inline int get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetInt(); }

    template<> inline rapidjson::Value::ConstArray get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetArray(); }

    template<> inline std::vector<std::string> get(const rapidjson::Value& aValue, const char* aName)
    {
        std::vector<std::string> result;
        const auto value = get<rapidjson::Value::ConstArray>(aValue, aName);
        std::transform(value.begin(), value.end(), std::back_inserter(result), [](const auto& elt) { return elt.GetString(); });
        return result;
    }

    template <typename Value> inline Value get(const rapidjson::Value& aValue, const char* aName, const Value& aDefault)
    {
        try {
            return get<Value>(aValue, aName);
        }
        catch (RapidjsonAssert&) {
            return aDefault;
        }
    }

// ----------------------------------------------------------------------

    class Object
    {
     public:
        inline Object(std::string aSrc) { mDoc.Parse(aSrc.c_str(), aSrc.size()); }
        inline Object(Object&& aSrc) : mDoc{std::move(aSrc.mDoc)} {}
        virtual inline ~Object() {}

        template <typename Value> inline Value get(const char* aName) const { return json_importer::get<Value>(mDoc, aName); }
        template <typename Value> inline Value get(const char* aName, const Value& aDefault) const { return json_importer::get(mDoc, aName, aDefault); }

        inline std::string get_string(const char* aName) const { return get(aName, std::string{}); }

     private:
        rapidjson::Document mDoc;

    }; // class Object

} // namespace json_importer

// ----------------------------------------------------------------------

class json_raw : public std::string
{
 public:
    using std::string::string;
};

namespace json_object_internal
{
    inline std::string make_value(std::string value) { return "\"" + value + "\""; }
    inline std::string make_value(json_raw value) { return value; }
    inline std::string make_value(const char* value) { return std::string{"\""} + value + "\""; }
    inline std::string make_value(bool value) { return value ? "true" : "false"; }
    inline std::string make_value(std::nullptr_t) { return "null"; }
    template <typename Value> inline typename std::enable_if<std::numeric_limits<Value>::is_integer, std::string>::type make_value(Value value) { return std::to_string(value); }
    template <typename Value> inline typename std::enable_if<std::is_floating_point<Value>::value, std::string>::type make_value(Value value) { return double_to_string(value); }

    template <typename Value> inline std::string add(std::string target, std::string key, Value value)
    {
        std::string result = target.size() > 2 ? target.substr(0, target.size() - 1) + "," : std::string{"{"};
        result += "\"" + key + "\":" + make_value(value) + "}";
        return result;
    }

    template <typename Value, typename ... Args> inline std::string add(std::string target, std::string key, Value value, Args ... args)
    {
        return add(add(target, key, value), args ...);
    }
}

template <typename ... Args> inline std::string json_object(Args ... args)
{
    return json_object_internal::add(std::string{}, args ...);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
