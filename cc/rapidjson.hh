#pragma once

#include <stdexcept>

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

template <typename Value> Value get(const rapidjson::Value& aValue, const char* aName);

template<> inline std::string get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetString(); }
template<> inline size_t get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetUint(); }
template<> inline unsigned get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetUint(); }
template<> inline int get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetInt(); }

template<> inline rapidjson::Value::ConstArray get(const rapidjson::Value& aValue, const char* aName) { return aValue[aName].GetArray(); }

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
