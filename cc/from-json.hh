#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>

#include "acmacs-base/string.hh"
#include "acmacs-base/rapidjson.hh"

// ----------------------------------------------------------------------

namespace from_json
{
    using ConstArray = rapidjson::Value::ConstArray;

    template <typename Value> Value get(const rapidjson::Value& aValue, const char* aName);

    template<> inline std::string get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetString(); }
    template<> inline size_t get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetUint(); }
    template<> inline unsigned get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetUint(); }
    template<> inline int get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetInt(); }
    template<> inline bool get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetBool(); }

    template<> inline ConstArray get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetArray(); }

    inline const char* get_string(const rapidjson::Value& aValue) { return aValue.GetString(); }
    inline std::string get_string_uppercase(const rapidjson::Value& aValue) { return string::upper(aValue.GetString()); }
    inline std::string get_string_lowercase(const rapidjson::Value& aValue) { return string::lower(aValue.GetString()); }

    template<> inline std::vector<std::string> get(const rapidjson::Value& aValue, const char* aName)
    {
        std::vector<std::string> result;
        const auto value = get<ConstArray>(aValue, aName);
          //std::transform(value.begin(), value.end(), std::back_inserter(result), [](const auto& elt) { return elt.GetString(); });
        std::transform(value.begin(), value.end(), std::back_inserter(result), &get_string);
        return result;
    }

    template <typename Value> inline Value get(const rapidjson::Value& aValue, const char* aName, const Value& aDefault)
    {
        try {
            return get<Value>(aValue, aName);
        }
        catch (rapidjson_assert&) {
            return aDefault;
        }
    }

// ----------------------------------------------------------------------

    class object
    {
     public:
        inline object(std::string aSrc) { mDoc.Parse(aSrc.c_str(), aSrc.size()); }
        inline object(object&& aSrc) : mDoc{std::move(aSrc.mDoc)} {}
        inline object(std::istream&& aInput, std::string aFilename = "istream")
            {
                if (!aInput)
                    throw std::runtime_error{"cannot read json input from " + aFilename };
                rapidjson::IStreamWrapper wrapper{aInput};
                mDoc.ParseStream(wrapper);
            }
        virtual inline ~object() {}

        template <typename Value> inline Value get(const char* aName) const { return from_json::get<Value>(mDoc, aName); }
        template <typename Value> inline Value get(const char* aName, const Value& aDefault) const { return from_json::get(mDoc, aName, aDefault); }

        inline std::string get_string(const char* aName) const { return get(aName, std::string{}); }
        inline ConstArray get_array(const char* aName) const { return from_json::get<ConstArray>(mDoc, aName); } // throws rapidjson_assert

        // inline const rapidjson::Document& doc() const { return mDoc; }

        inline std::string to_json() const
            {
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<decltype(buffer)> writer(buffer);
                mDoc.Accept(writer);
                return buffer.GetString();
            }

     private:
        rapidjson::Document mDoc;

    }; // class Object

} // namespace from_json

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
