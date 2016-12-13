#pragma once

#include <iostream>
#include <stdexcept>
#include <vector>
#include <map>
#include <stack>
#include <typeinfo>
#include <memory>

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"

#include "acmacs-base/read-file.hh"

// ----------------------------------------------------------------------

namespace json_reader
{
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    class Failure : public std::runtime_error { public: using std::runtime_error::runtime_error; inline Failure() : std::runtime_error{""} {} };
    class Pop : public std::exception { public: using std::exception::exception; };

// ----------------------------------------------------------------------

    template <typename Target> class HandlerBase
    {
     public:
        inline HandlerBase(Target& aTarget) : mTarget(aTarget), mIgnore(false) {}
        virtual ~HandlerBase() {}

        inline virtual HandlerBase* StartObject() { throw Failure(std::string("HandlerBase StartObject ") + typeid(*this).name()); }
        inline virtual HandlerBase* EndObject() { throw Pop(); }
        inline virtual HandlerBase* StartArray() { throw Failure(std::string("HandlerBase StartArray ") + typeid(*this).name()); }
        inline virtual HandlerBase* EndArray() { throw Pop(); }
        inline virtual HandlerBase* Double(double d) { throw Failure("HandlerBase Double " + std::to_string(d)); }
        inline virtual HandlerBase* Int(int i) { throw Failure("HandlerBase Int " + std::to_string(i)); }
        inline virtual HandlerBase* Uint(unsigned u) { throw Failure("HandlerBase Uint " + std::to_string(u)); }

        inline virtual HandlerBase* Key(const char* str, rapidjson::SizeType length)
            {
                if ((length == 1 && *str == '_') || (length > 0 && *str == '?'))
                    mIgnore = true;
                else
                    throw Failure("HandlerBase Key: \"" + std::string(str, length) + "\"");
                return nullptr;
            }

        inline virtual HandlerBase* String(const char* str, rapidjson::SizeType length)
            {
                if (mIgnore)
                    mIgnore = false;
                else
                    throw Failure("HandlerBase String: \"" + std::string(str, length) + "\"");
                return nullptr;
            }

     protected:
        Target& mTarget;
        bool mIgnore;
    };

// ----------------------------------------------------------------------

    template <typename Target, typename Element, typename ElementHandler> class ListHandler : public HandlerBase<Target>
    {
     public:
        inline ListHandler(Target& aTarget, std::vector<Element>& aList)
            : HandlerBase<Target>(aTarget), mList(aList), mStarted(false) {}

        inline virtual HandlerBase<Target>* StartArray()
            {
                if (mStarted)
                    throw Failure();
                mStarted = true;
                return nullptr;
            }

        inline virtual HandlerBase<Target>* StartObject()
            {
                if (!mStarted)
                    throw Failure();
                mList.emplace_back();
                return new ElementHandler(HandlerBase<Target>::mTarget, mList.back());
            }

        inline virtual HandlerBase<Target>* EndObject() { throw Failure(); }

     private:
        std::vector<Element>& mList;
        bool mStarted;

    }; // class StringListHandler

// ----------------------------------------------------------------------

    template <typename Target> class StringListHandler : public HandlerBase<Target>
    {
     public:
        inline StringListHandler(Target& aTarget, std::vector<std::string>& aList)
            : HandlerBase<Target>(aTarget), mList(aList), mStarted(false) {}

        inline virtual HandlerBase<Target>* StartArray()
            {
                if (mStarted)
                    throw Failure();
                mStarted = true;
                return nullptr;
            }

        inline virtual HandlerBase<Target>* EndObject() { throw Failure(); }

        inline virtual HandlerBase<Target>* String(const char* str, rapidjson::SizeType length)
            {
                mList.emplace_back(str, length);
                return nullptr;
            }

     private:
        std::vector<std::string>& mList;
        bool mStarted;

    }; // class StringListHandler

// ----------------------------------------------------------------------

    template <typename Target> class MapListHandler : public HandlerBase<Target>
    {
     public:
        inline MapListHandler(Target& aTarget, std::map<std::string, std::vector<std::string>>& aMap)
            : HandlerBase<Target>{aTarget}, mMap(aMap), mStarted(false) {}

        inline virtual HandlerBase<Target>* StartObject()
            {
                if (mStarted)
                    throw Failure();
                mStarted = true;
                return nullptr;
            }

        inline virtual HandlerBase<Target>* EndArray() { throw Failure(); }

        inline virtual HandlerBase<Target>* Key(const char* str, rapidjson::SizeType length)
            {
                return new StringListHandler<Target>(HandlerBase<Target>::mTarget, mMap[{str, length}]);
            }

     private:
        std::map<std::string, std::vector<std::string>>& mMap;
        bool mStarted;

    }; // class MapListHandler

// ----------------------------------------------------------------------

    template <typename Target> class StringMappingHandler : public HandlerBase<Target>
    {
     public:
        inline StringMappingHandler(Target& aTarget, std::vector<std::pair<std::string, std::string>>& aMapping)
            : HandlerBase<Target>{aTarget}, mMapping(aMapping), mStarted(false) {}

        inline virtual HandlerBase<Target>* StartObject()
            {
                if (mStarted)
                    throw json_reader::Failure();
                mStarted = true;
                return nullptr;
            }

        inline virtual HandlerBase<Target>* Key(const char* str, rapidjson::SizeType length)
            {
                mKey.assign(str, length);
                return nullptr;
            }

        inline virtual HandlerBase<Target>* String(const char* str, rapidjson::SizeType length)
            {
                if (mKey.empty())
                    throw json_reader::Failure();
                mMapping.emplace_back(mKey, std::string(str, length));
                mKey.erase();
                return nullptr;
            }

     private:
        std::vector<std::pair<std::string, std::string>>& mMapping;
        bool mStarted;
        std::string mKey;

    }; // class StringMappingHandler

// ----------------------------------------------------------------------

    template <typename Target, typename RootHandler> class DocRootHandler : public HandlerBase<Target>
    {
     public:
        inline DocRootHandler(Target& aTarget) : HandlerBase<Target>(aTarget) {}

        inline virtual HandlerBase<Target>* StartObject() { return new RootHandler(HandlerBase<Target>::mTarget); }
    };

// ----------------------------------------------------------------------

    template <typename Target, typename RootHandler> class ReaderEventHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, ReaderEventHandler<Target, RootHandler>>
    {
     private:
        template <typename... Args> inline bool handler(HandlerBase<Target>* (HandlerBase<Target>::*aHandler)(Args... args), Args... args)
            {
                try {
                    auto new_handler = ((*mHandler.top()).*aHandler)(args...);
                    if (new_handler)
                        mHandler.emplace(new_handler);
                }
                catch (Pop&) {
                    if (mHandler.empty())
                        return false;
                    mHandler.pop();
                }
                catch (Failure& err) {
                    if (*err.what())
                        std::cerr << "ERROR: " << err.what() << std::endl;
                    return false;
                }
                return true;
            }

     public:
        inline ReaderEventHandler(Target& aTarget)
            : mTarget(aTarget)
            {
                mHandler.emplace(new DocRootHandler<Target, RootHandler>(mTarget));
            }

        inline bool StartObject() { return handler(&HandlerBase<Target>::StartObject); }
        inline bool EndObject(rapidjson::SizeType /*memberCount*/) { return handler(&HandlerBase<Target>::EndObject); }
        inline bool StartArray() { return handler(&HandlerBase<Target>::StartArray); }
        inline bool EndArray(rapidjson::SizeType /*elementCount*/) { return handler(&HandlerBase<Target>::EndArray); }
        inline bool Key(const char* str, rapidjson::SizeType length, bool /*copy*/) { return handler(&HandlerBase<Target>::Key, str, length); }
        inline bool String(const char* str, rapidjson::SizeType length, bool /*copy*/) { return handler(&HandlerBase<Target>::String, str, length); }
        inline bool Int(int i) { return handler(&HandlerBase<Target>::Int, i); }
        inline bool Uint(unsigned u) { return handler(&HandlerBase<Target>::Uint, u); }
        inline bool Double(double d) { return handler(&HandlerBase<Target>::Double, d); }

          // inline bool Bool(bool /*b*/) { return false; }
          // inline bool Null() { std::cout << "Null()" << std::endl; return false; }
          // inline bool Int64(int64_t i) { std::cout << "Int64(" << i << ")" << std::endl; return false; }
          // inline bool Uint64(uint64_t u) { std::cout << "Uint64(" << u << ")" << std::endl; return false; }

     private:
        Target& mTarget;
        std::stack<std::unique_ptr<HandlerBase<Target>>> mHandler;
    };

// ----------------------------------------------------------------------

    template <typename Target, typename RootHandler> inline void read_from_file(std::string aFilename, Target& aTarget)
    {
        const std::string buffer = aFilename == "-" ? acmacs_base::read_stdin() : acmacs_base::read_file(aFilename);
        if (buffer[0] == '{') {
            ReaderEventHandler<Target, RootHandler> handler{aTarget};
            rapidjson::Reader reader;
            rapidjson::StringStream stream{buffer.c_str()};
            reader.Parse(stream, handler);
            if (reader.HasParseError())
                throw Error("cannot read " + aFilename + ": data parsing failed at pos " + std::to_string(reader.GetErrorOffset()) + ": " +  GetParseError_En(reader.GetParseErrorCode()) + "\n" + buffer.substr(reader.GetErrorOffset(), 50));
        }
        else
            throw json_reader::Error("cannot read " + aFilename + ": unrecognized source format");
    }

// ----------------------------------------------------------------------

} // namespace json_reader

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End: