#pragma once

#include <iostream>
#include <stdexcept>
#include <vector>
#include <map>
#include <stack>

#include "rapidjson/reader.h"

// ----------------------------------------------------------------------

namespace json_reader
{
    class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    class Failure : public std::exception { public: using std::exception::exception; };
    class Pop : public std::exception { public: using std::exception::exception; };

// ----------------------------------------------------------------------

    template <typename Target> class HandlerBase
    {
     public:
        inline HandlerBase(Target& aTarget) : mTarget(aTarget), mIgnore(false) {}
        virtual ~HandlerBase() {}

        inline virtual HandlerBase* StartObject() { std::cerr << "HandlerBase StartObject " << typeid(*this).name() << std::endl; throw Failure(); }
        inline virtual HandlerBase* EndObject() { throw Pop(); }
        inline virtual HandlerBase* StartArray() { throw Failure(); }
        inline virtual HandlerBase* EndArray() { throw Pop(); }
        inline virtual HandlerBase* Double(double d) { std::cerr << "Double: " << d << std::endl; throw Failure(); }
        inline virtual HandlerBase* Int(int i) { std::cerr << "Int: " << i << std::endl; throw Failure(); }
        inline virtual HandlerBase* Uint(unsigned u) { std::cerr << "Uint: " << u << std::endl; throw Failure(); }

        inline virtual HandlerBase* Key(const char* str, rapidjson::SizeType length)
            {
                if ((length == 1 && *str == '_') || (length > 0 && *str == '?')) {
                    mIgnore = true;
                }
                else {
                    std::cerr << "Key: \"" << std::string(str, length) << '"' << std::endl;
                    throw Failure();
                }
                return nullptr;
            }

        inline virtual HandlerBase* String(const char* str, rapidjson::SizeType length)
            {
                if (mIgnore) {
                    mIgnore = false;
                }
                else {
                    std::cerr << "String: \"" << std::string(str, length) << '"' << std::endl;
                    throw Failure();
                }
                return nullptr;
            }

     protected:
        Target& mTarget;
        bool mIgnore;
    };

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
            : HandlerBase<Target>(aTarget), mMap(aMap), mStarted(false) {}

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
                catch (Failure&) {
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

} // namespace json_reader

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
