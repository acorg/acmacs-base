#pragma once

#include <iostream>
#include <stdexcept>
#include <vector>
#include <map>
#include <stack>
#include <typeinfo>
#include <memory>

#include "acmacs-base/fmt.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/rapidjson.hh"

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
        inline virtual HandlerBase* Double(double d) { if (mIgnore) { mIgnore = false; return nullptr; } throw Failure("HandlerBase Double " + std::to_string(d)); }
        inline virtual HandlerBase* Int(int i) { if (mIgnore) { mIgnore = false; return nullptr; } throw Failure("HandlerBase Int " + std::to_string(i)); }
        inline virtual HandlerBase* Uint(unsigned u) { if (mIgnore) { mIgnore = false; return nullptr; } throw Failure("HandlerBase Uint " + std::to_string(u)); }
        inline virtual HandlerBase* Bool(bool b) { if (mIgnore) { mIgnore = false; return nullptr; } throw Failure("HandlerBase Bool " + std::to_string(b)); }
        inline virtual HandlerBase* Null() { if (mIgnore) { mIgnore = false; return nullptr; } throw Failure("HandlerBase Null"); }

        inline virtual HandlerBase* Key(const char* str, rapidjson::SizeType length)
            {
                if ((length == 1 && *str == '_') || (length > 0 && (*str == '?' || str[length - 1] == '?')))
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

    template <typename Target> class GenericListHandler : public HandlerBase<Target>
    {
     public:
        inline GenericListHandler(Target& aTarget, size_t aExpectedSize)
            : HandlerBase<Target>(aTarget), mStarted(false), mExpectedSize(aExpectedSize) {}

        inline virtual HandlerBase<Target>* StartArray()
            {
                if (mStarted)
                    throw Failure{};
                mStarted = true;
                return nullptr;
            }

        inline virtual HandlerBase<Target>* EndArray()
            {
                if (mExpectedSize && size() != mExpectedSize)
                    throw Failure{"Unexpected resulting list size: " + std::to_string(size()) + " expected: " + std::to_string(mExpectedSize)};
                throw Pop();
            }

        inline virtual HandlerBase<Target>* EndObject() { throw Failure(); }

     protected:
        inline bool started() const { return mStarted; }
        virtual size_t size() const = 0;

     private:
        bool mStarted;
        size_t mExpectedSize;

    }; // class GenericListHandler

// ----------------------------------------------------------------------

    template <typename Target, typename Element, typename ElementHandler> class ListHandler : public GenericListHandler<Target>
    {
     public:
        inline ListHandler(Target& aTarget, std::vector<Element>& aList, size_t aExpectedSize = 0)
            : GenericListHandler<Target>(aTarget, aExpectedSize), mList(aList) {}

        inline virtual HandlerBase<Target>* StartObject()
            {
                if (!this->started())
                    throw Failure{};
                mList.emplace_back();
                return new ElementHandler(HandlerBase<Target>::mTarget, mList.back());
            }

     protected:
        virtual inline size_t size() const { return mList.size(); }

     private:
        std::vector<Element>& mList;

    }; // class ListHandler

// ----------------------------------------------------------------------

    template <typename Target> class StringListHandler : public GenericListHandler<Target>
    {
     public:
        inline StringListHandler(Target& aTarget, std::vector<std::string>& aList, size_t aExpectedSize = 0)
            : GenericListHandler<Target>(aTarget, aExpectedSize), mList(aList) {}

        inline virtual HandlerBase<Target>* String(const char* str, rapidjson::SizeType length)
            {
                mList.emplace_back(str, length);
                return nullptr;
            }

     protected:
        virtual inline size_t size() const { return mList.size(); }

     private:
        std::vector<std::string>& mList;

    }; // class StringListHandler

// ----------------------------------------------------------------------

    template <typename Target> class UintListHandler : public GenericListHandler<Target>
    {
     public:
        inline UintListHandler(Target& aTarget, std::vector<size_t>& aList, size_t aExpectedSize = 0)
            : GenericListHandler<Target>(aTarget, aExpectedSize), mList(aList) {}

        inline virtual HandlerBase<Target>* Uint(unsigned u)
            {
                mList.push_back(u);
                return nullptr;
            }

     protected:
        virtual inline size_t size() const { return mList.size(); }

     private:
        std::vector<size_t>& mList;

    }; // class UintListHandler

// ----------------------------------------------------------------------

    template <typename Target> class DoubleListHandler : public GenericListHandler<Target>
    {
     public:
        inline DoubleListHandler(Target& aTarget, std::vector<double>& aList, size_t aExpectedSize = 0)
            : GenericListHandler<Target>(aTarget, aExpectedSize), mList(aList) {}

        inline virtual HandlerBase<Target>* Double(double d)
            {
                mList.push_back(d);
                return nullptr;
            }

     protected:
        virtual inline size_t size() const { return mList.size(); }

     private:
        std::vector<double>& mList;

    }; // class DoubleListHandler

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
        inline bool Bool(bool b) { return handler(&HandlerBase<Target>::Bool, b); }
        inline bool Null() { return handler(&HandlerBase<Target>::Null); }
        inline bool Int64(int64_t i) { std::cerr << "ReaderEventHandler::Int64(" << i << ")" << std::endl; return false; }
        inline bool Uint64(uint64_t u) { std::cerr << "ReaderEventHandler::Uint64(" << u << ")" << std::endl; return false; }

     private:
        Target& mTarget;
        std::stack<std::unique_ptr<HandlerBase<Target>>> mHandler;
    };

// ----------------------------------------------------------------------

    template <typename Target, typename RootHandler> inline void read_from_file(std::string_view aFilename, Target& aTarget)
    {
        const std::string buffer = aFilename == "-" ? acmacs::file::read_stdin() : static_cast<std::string>(acmacs::file::read(aFilename));
        if (buffer[0] == '{') {
            ReaderEventHandler<Target, RootHandler> handler{aTarget};
            rapidjson::Reader reader;
            rapidjson::StringStream stream{buffer.c_str()};
            reader.Parse(stream, handler);
            if (reader.HasParseError())
                throw Error{fmt::format("cannot read {}: data parsing failed at pos {}: {}\n{}", aFilename, reader.GetErrorOffset(), GetParseError_En(reader.GetParseErrorCode()), buffer.substr(reader.GetErrorOffset(), 50))};
        }
        else
            throw json_reader::Error{fmt::format("cannot read {}: unrecognized source format", aFilename)};
    }

// ----------------------------------------------------------------------

} // namespace json_reader

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
