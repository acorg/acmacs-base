#pragma once

// ----------------------------------------------------------------------

namespace from_json
{
    class rapidjson_assert : public std::exception { public: using std::exception::exception; };
}

#ifdef RAPIDJSON_ASSERT
#undef RAPIDJSON_ASSERT
#endif
#define RAPIDJSON_ASSERT(x) {if (!(x)) throw from_json::rapidjson_assert{}; }

// ----------------------------------------------------------------------

#pragma GCC diagnostic push

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"

#ifdef RAPIDJSON_ACCESS_WRITER
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#endif

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
