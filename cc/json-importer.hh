#pragma once

#include <string>
#include <vector>
#include <map>
#include <stack>
#include <typeinfo>
#include <functional>

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"

// ----------------------------------------------------------------------

namespace json_importer
{
    class EventHandler;

    namespace storers {

        class Base
        {
         protected:
            class Failure : public std::runtime_error { public: using std::runtime_error::runtime_error; inline Failure() : std::runtime_error{""} {} };
            class Pop : public std::exception { public: using std::exception::exception; };
            friend class json_importer::EventHandler;

         public:
            virtual ~Base() {}

            inline virtual Base* StartObject() { throw Failure(typeid(*this).name() + std::string("::StartObject")); }
            inline virtual Base* EndObject() { throw Failure(typeid(*this).name() + std::string("::EndObject")); } // { throw Pop(); }
            inline virtual Base* StartArray() { throw Failure(typeid(*this).name() + std::string("::StartArray")); }
            inline virtual Base* EndArray() { throw Failure(typeid(*this).name() + std::string("::EndArray")); } // { throw Pop(); }
            inline virtual Base* Key(const char* str, rapidjson::SizeType length) { throw Failure(typeid(*this).name() + std::string("::Key \"") + std::string(str, length) + "\""); }
            inline virtual Base* String(const char* str, rapidjson::SizeType length) { throw Failure(typeid(*this).name() + std::string("::String \"") + std::string(str, length) + "\""); }
            inline virtual Base* Int(int i) { throw Failure(typeid(*this).name() + std::string("::Int ") + std::to_string(i)); }
            inline virtual Base* Uint(unsigned u) { throw Failure(typeid(*this).name() + std::string("::Uint ") + std::to_string(u)); }
            inline virtual Base* Double(double d) { throw Failure(typeid(*this).name() + std::string("::Double ") + std::to_string(d)); }
            inline virtual Base* Bool(bool b) { throw Failure(typeid(*this).name() + std::string("::Bool ") + std::to_string(b)); }
            inline virtual Base* Null() { throw Failure(typeid(*this).name() + std::string("::Null")); }
            inline virtual Base* Int64(int64_t i) { throw Failure(typeid(*this).name() + std::string("::Int64 ") + std::to_string(i)); }
            inline virtual Base* Uint64(uint64_t u) { throw Failure(typeid(*this).name() + std::string("::Uint64 ") + std::to_string(u)); }
        };

        template <typename F> class Storer : public Base
        {
         public:
            inline Storer(F aStorage) : mStorage(aStorage) {}
         protected:
            inline virtual void pop() {}
            template <typename ...Args> inline Base* store(Args... args) { mStorage(args...); pop(); return nullptr; }
            inline F& storage() { return mStorage; }
         private:
            F mStorage;
        };

        template <typename F> class StringLength : public Storer<F>
        {
         public:
            using Storer<F>::Storer;
            inline virtual Base* String(const char* str, rapidjson::SizeType length) { return this->store(str, length); }
        };

        template <typename F> class Double_ : public Storer<F>
        {
         public:
            using Storer<F>::Storer;
            inline virtual Base* Double(double d) { return this->store(d); }
        };

        template <typename F> class Unsigned_ : public Storer<F>
        {
         public:
            using Storer<F>::Storer;
            inline virtual Base* Uint(unsigned u) { return this->store(u); }
        };

        template <typename F> class Int_ : public Storer<F>
        {
         public:
            using Storer<F>::Storer;
            inline virtual Base* Int(int i) { return this->store(i); }
            inline virtual Base* Uint(unsigned u) { return Int(static_cast<int>(u)); }
        };

        template <typename F> class Bool_ : public Storer<F>
        {
         public:
            using Storer<F>::Storer;
            inline virtual Base* Bool(bool b) { return this->store(b); }
              // inline virtual Base* Int(int i) { return Bool(i != 0); }
            inline virtual Base* Uint(unsigned u) { return Bool(u != 0); }
        };

          // ----------------------------------------------------------------------
          // Type detector functions
          // They are never called but used by field(std::vector<Field>& (Parent::*accessor)()) and reader(void(T::*setter)(V), T& target) functions below to infer of the storer's type
          // ----------------------------------------------------------------------

        template <typename F> inline Unsigned_<F> type_detector(size_t) { throw std::exception{}; }
        template <typename F> inline Unsigned_<F> type_detector(unsigned) { throw std::exception{}; }
        template <typename F> inline Int_<F> type_detector(int) { throw std::exception{}; }
        template <typename F> inline Double_<F> type_detector(double) { throw std::exception{}; }
        template <typename F> inline Bool_<F> type_detector(bool) { throw std::exception{}; }
        template <typename F> inline StringLength<F> type_detector(std::string) { throw std::exception{}; }

          // ----------------------------------------------------------------------
          // to be used as template parameter F for the above to store Array values
          // ----------------------------------------------------------------------

        template <typename Target> class ArrayElement
        {
         public:
            inline ArrayElement(std::vector<Target>& aTarget) : mTarget(aTarget) {}
            // inline void operator()(Target aValue) { mTarget.emplace_back(aValue); }
            // inline void operator()(const char* str, size_t length) { mTarget.emplace_back(str, length); }
            template <typename ...Args> inline void operator()(Args ...args) { mTarget.emplace_back(args...); }
         private:
            std::vector<Target>& mTarget;
        };

        template <typename Target> class ArrayOfArrayElementTarget
        {
         public:
            inline ArrayOfArrayElementTarget(Target& aTarget) : mTarget(aTarget) {}
              // inline void operator()(Target aValue) { mTarget.back().emplace_back(aValue); }
            template <typename ...Args> inline void operator()(Args ...args) { mTarget.back().emplace_back(args...); }
            inline size_t size() const { return mTarget.size(); }
            inline void clear() { mTarget.clear(); }
            inline void new_nested() { mTarget.emplace_back(); }
         private:
            Target& mTarget;
        };

        template <typename Target> using ArrayOfArrayElement = ArrayOfArrayElementTarget<std::vector<std::vector<Target>>>;

    } // namespace storers

      // ----------------------------------------------------------------------

    namespace readers {

        using Base = storers::Base;

          // ----------------------------------------------------------------------
          // reader: Object
          // ----------------------------------------------------------------------

        template <typename Target> class Object : public Base
        {
         public:
            inline Object(Target aTarget, bool aStarted = false) : mTarget(aTarget), mStarted(aStarted) {}

            inline virtual Base* Key(const char* str, rapidjson::SizeType length)
                {
                    if (!mStarted)
                        throw Failure(typeid(*this).name() + std::string(": unexpected Key event"));
                    Base* r = match_key(str, length);
                    if (!r) {
                          // support for keys starting or ending with ? and "_"
                        r = Base::Key(str, length);
                    }
                      // else
                      //     std::cerr << "readers::Object " << std::string(str, length) << " -> PUSH " << typeid(*r).name() << std::endl;
                    return r;
                }

            inline virtual Base* StartObject()
                {
                    if (mStarted)
                        throw Failure(typeid(*this).name() + std::string(": unexpected StartObject event"));
                    mStarted = true;
                    return nullptr;
                }

            inline virtual Base* EndObject()
                {
                    throw Pop();
                }

         protected:
            virtual Base* match_key(const char* str, rapidjson::SizeType length) = 0;

            inline Target& target() { return mTarget; }

         private:
            Target mTarget;
            bool mStarted;
        };

          // ----------------------------------------------------------------------
          // reader: value storer
          // ----------------------------------------------------------------------

        template <typename ValueStorer> class Value : public ValueStorer
        {
         public:
            using ValueStorer::ValueStorer;
         protected:
            inline virtual void pop() { throw storers::Base::Pop(); }
        };

          // ----------------------------------------------------------------------
          // reader maker base
          // ----------------------------------------------------------------------

        namespace makers
        {
            template <typename Parent> class Base
            {
             public:
                inline Base() = default;
                virtual ~Base() {}
                virtual readers::Base* reader(Parent& parent) = 0;
            };

        } // namespace makers

          // ----------------------------------------------------------------------
          // Structure to keep object reader description
          // ----------------------------------------------------------------------

        template <typename Parent> using data = std::map<std::string,std::shared_ptr<makers::Base<Parent>>>; // cannot have unique_ptr here because std::map requires copying

          // ----------------------------------------------------------------------
          // reader: DataRef
          // ----------------------------------------------------------------------

        template <typename Target> class DataRef : public Object<Target&>
        {
         public:
            inline DataRef(Target& aTarget, data<Target>& aData, bool aStarted = false) : Object<Target&>(aTarget, aStarted), mData(aData) {}

         protected:
            virtual inline readers::Base* match_key(const char* str, rapidjson::SizeType length)
                {
                    const std::string k{str, length};
                      // std::cerr << typeid(*this).name() << " " << k << std::endl;
                    auto e = mData.find(k);
                    if (e != mData.end())
                          // return e->second(this->target());
                        return e->second->reader(this->target());
                    return nullptr;
                }

         private:
            data<Target>& mData;

        }; // class DataRef<Target>

          // ----------------------------------------------------------------------
          // reader: ArrayOfObjects
          // ----------------------------------------------------------------------

        template <typename Element> class ArrayOfObjects : public Base
        {
         public:
            inline ArrayOfObjects(std::vector<Element>& aArray, data<Element>& aData) : mArray(aArray), mData(aData), mStarted(false) {}

            inline virtual Base* StartArray()
                {
                    if (mStarted)
                        throw Failure(typeid(*this).name() + std::string(": unexpected StartArray event"));
                    mStarted = true;
                    mArray.clear(); // erase all old elements
                    return nullptr;
                }

            inline virtual Base* EndArray()
                {
                      // std::cerr << "EndArray of " << typeid(Element).name() << " elements:" << mArray.size() << std::endl;
                    throw Pop();
                }

            inline virtual Base* StartObject()
                {
                    if (!mStarted)
                        throw Failure(typeid(*this).name() + std::string(": unexpected StartObject event"));
                    mArray.emplace_back();
                    return new DataRef<Element>(mArray.back(), mData, true);
                }

         private:
            std::vector<Element>& mArray;
            data<Element>& mData;
            bool mStarted;

        }; // class ArrayOfObjects<Element>

          // ----------------------------------------------------------------------
          // reader: ArrayOfValues
          // ----------------------------------------------------------------------

        template <typename Element, typename Storer> class ArrayOfValues : public Storer
        {
         public:
            inline ArrayOfValues(std::vector<Element>& aArray) : Storer(aArray), mArray(aArray), mStarted(false) {}

            inline virtual Base* StartArray()
                {
                    if (mStarted)
                        throw Base::Failure(typeid(*this).name() + std::string(": unexpected StartArray event"));
                    mStarted = true;
                    mArray.clear(); // erase all old elements
                    return nullptr;
                }

            inline virtual Base* EndArray()
                {
                    throw Base::Pop();
                }

         private:
            std::vector<Element>& mArray;
            bool mStarted;

        }; // class ArrayOfValues<Element>

          // ----------------------------------------------------------------------
          // reader: ArrayOfArrayOfValues
          // ----------------------------------------------------------------------

        template <typename Target, typename Storer> class ArrayOfArrayOfValuesTarget : public Storer
        {
         public:
            inline ArrayOfArrayOfValuesTarget(Target& aArray) : Storer(aArray), mNesting(0) {}

            inline virtual Base* StartArray()
                {
                    switch (mNesting) {
                      case 0:
                          this->storage().clear(); // erase all old elements
                          break;
                      case 1:
                          this->storage().new_nested();
                          break;
                      default:
                          throw Base::Failure(typeid(*this).name() + std::string(": unexpected StartArray event"));
                    }
                    ++mNesting;
                    return nullptr;
                }

            inline virtual Base* EndArray()
                {
                    switch (mNesting) {
                      case 1:
                          throw Base::Pop();
                      case 2:
                          break;
                      default:
                          throw Base::Failure(typeid(*this).name() + std::string(": internal, EndArray event with nesting ") + std::to_string(mNesting));
                    }
                    --mNesting;
                    return nullptr;
                }

         private:
            size_t mNesting;

        }; // class ArrayOfArrayOfValuesTarget<Target, Storer>

        template <typename Element, typename Storer> using ArrayOfArrayOfValues = ArrayOfArrayOfValuesTarget<std::vector<std::vector<Element>>, Storer>;

          // ----------------------------------------------------------------------
          // reader: template helpers
          // ----------------------------------------------------------------------

        template <typename T> inline Base* reader(void(T::*setter)(const char*, size_t), T& target)
        {
            using Bind = decltype(std::bind(setter, &target, std::placeholders::_1, std::placeholders::_2));
            return new Value<storers::StringLength<Bind>>(std::bind(setter, &target, std::placeholders::_1, std::placeholders::_2));
        }

        template <typename T, typename V> inline Base* reader(void(T::*setter)(V), T& target)
        {
            using Bind = decltype(std::bind(setter, &target, std::placeholders::_1));
            using Storer = decltype(storers::type_detector<Bind>(std::declval<V>()));
            return new Value<Storer>(std::bind(setter, &target, std::placeholders::_1));
        }

          //   // for readers::Object<> derivatives, e.g. return readers::reader<JsonReaderChart>(&Ace::chart, target());
          // template <template<typename> class Reader, typename Parent, typename Field> inline Base* reader(Field& (Parent::*accessor)(), Parent& parent)
          // {
          //     using Bind = decltype(std::bind(std::declval<Field& (Parent::*&)()>(), std::declval<Parent*>()));
          //     return new Reader<Bind>(std::bind(accessor, &parent));
          // }

          // ----------------------------------------------------------------------
          // reader makers
          // ----------------------------------------------------------------------

        namespace makers
        {
            template <typename Parent, typename Func>
                class Setter : public Base<Parent>
            {
             public:
                inline Setter(Func aF) : mF(aF) {}
                virtual inline readers::Base* reader(Parent& parent) { return readers::reader(mF, parent); }

             private:
                Func mF;
            };

            template <typename Parent, typename Field, typename Func>
                class Accessor : public Base<Parent>
            {
             public:
                inline Accessor(Func aF, data<Field>& aData) : mF(aF), mData(aData) {}
                virtual inline readers::Base* reader(Parent& parent)
                    {
                        return new DataRef<Field>(std::bind(mF, &parent)(), mData);
                    }

             private:
                Func mF;
                data<Field>& mData;
            };

            template <typename Parent, typename Element, typename Func>
                class ArrayOfObjectsAccessor : public Base<Parent>
            {
             public:
                inline ArrayOfObjectsAccessor(Func aF, data<Element>& aData) : mF(aF), mData(aData) {}
                virtual inline readers::Base* reader(Parent& parent)
                    {
                        return new ArrayOfObjects<Element>(std::bind(mF, &parent)(), mData);
                    }

             private:
                Func mF;
                data<Element>& mData;
            };

            template <typename Parent, typename Element, typename Func, typename Storer>
                class ArrayOfValuesAccessor : public Base<Parent>
            {
             public:
                inline ArrayOfValuesAccessor(Func aF) : mF(aF) {}
                virtual inline readers::Base* reader(Parent& parent)
                    {
                        return new ArrayOfValues<Element, Storer>(std::bind(mF, &parent)());
                    }

             private:
                Func mF;
            };

            template <typename Parent, typename Target, typename Func, typename Storer>
                class ArrayOfArrayOfValuesTargetAccessor : public Base<Parent>
            {
             public:
                inline ArrayOfArrayOfValuesTargetAccessor(Func aF) : mF(aF) {}
                virtual inline readers::Base* reader(Parent& parent)
                    {
                        return new ArrayOfArrayOfValuesTarget<Target, Storer>(std::bind(mF, &parent)());
                    }

             private:
                Func mF;
            };

            template <typename Parent, typename Element, typename Func, typename Storer>
                class ArrayOfArrayOfValuesAccessor : public Base<Parent>
            {
             public:
                inline ArrayOfArrayOfValuesAccessor(Func aF) : mF(aF) {}
                virtual inline readers::Base* reader(Parent& parent)
                    {
                        return new ArrayOfArrayOfValues<Element, Storer>(std::bind(mF, &parent)());
                    }

             private:
                Func mF;
            };

            template <typename Storer, typename Parent, typename Field>
                class GenericAccessor : public Base<Parent>
            {
             public:
                using Accessor  = Field& (Parent::*)();
                inline GenericAccessor(Accessor aAccessor) : mAccessor(aAccessor) {}
                virtual inline readers::Base* reader(Parent& parent)
                    {
                        return new Storer(std::bind(mAccessor, &parent)());
                    }
             private:
                Accessor mAccessor;
            };

        } // namespace makers

          // ----------------------------------------------------------------------

    } // namespace readers

      // ----------------------------------------------------------------------
      // Rapidjson event handler
      // ----------------------------------------------------------------------

    class EventHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, EventHandler>
    {
     public:
          // template <typename Target> inline EventHandler(Target& aTarget)
          //     {
          //         mHandler.emplace(json_reader(aTarget));
          //     }

        template <typename Target> inline EventHandler(Target& aTarget, readers::data<Target>& aData)
            {
                mHandler.emplace(new readers::DataRef<Target>(aTarget, aData));
            }

     private:
        template <typename... Args> inline bool handler(readers::Base* (readers::Base::*aHandler)(Args... args), Args... args)
            {
                try {
                    auto new_handler = ((*mHandler.top()).*aHandler)(args...);
                    if (new_handler)
                        mHandler.emplace(new_handler);
                }
                catch (readers::Base::Pop&) {
                    if (mHandler.empty())
                        return false;
                    mHandler.pop();
                }
                catch (readers::Base::Failure& err) {
                    if (*err.what())
                        std::cerr << "ERROR: " << err.what() << std::endl;
                    return false;
                }
                  // catch (std::exception& err) {
                  //     std::cerr << "ERROR: " << err.what() << std::endl;
                  //     return false;
                  // }
                return true;
            }

     public:
        inline bool StartObject() { return handler(&readers::Base::StartObject); }
        inline bool EndObject(rapidjson::SizeType /*memberCount*/) { return handler(&readers::Base::EndObject); }
        inline bool StartArray() { return handler(&readers::Base::StartArray); }
        inline bool EndArray(rapidjson::SizeType /*elementCount*/) { return handler(&readers::Base::EndArray); }
        inline bool Key(const char* str, rapidjson::SizeType length, bool /*copy*/) { return handler(&readers::Base::Key, str, length); }
        inline bool String(const char* str, rapidjson::SizeType length, bool /*copy*/) { return handler(&readers::Base::String, str, length); }
        inline bool Int(int i) { return handler(&readers::Base::Int, i); }
        inline bool Uint(unsigned u) { return handler(&readers::Base::Uint, u); }
        inline bool Double(double d) { return handler(&readers::Base::Double, d); }
        inline bool Bool(bool b) { return handler(&readers::Base::Bool, b); }
        inline bool Null() { return handler(&readers::Base::Null); }
        inline bool Int64(int64_t i) { return handler(&readers::Base::Int64, i); }
        inline bool Uint64(uint64_t u) { return handler(&readers::Base::Uint64, u); }

     private:
        std::stack<std::unique_ptr<readers::Base>> mHandler;

    }; // class EventHandler<>

// ======================================================================
// Exports
// ======================================================================

    template <typename Parent> using data = readers::data<Parent>;

      // Base class for custom Storers
    using StorerBase = storers::Base;

      // Field is a simple object set via setter: void Parent::setter(const Value& value)
    template <typename Parent, typename ...Args> inline std::shared_ptr<readers::makers::Base<Parent>> field(void (Parent::*setter)(Args...))
    {
        return std::make_shared<readers::makers::Setter<Parent, decltype(setter)>>(setter);
    }

      // Field is a simple object set via setter: void ParentBase::setter(const Value& value)
      // Parent is derived from ParentBase, setter declared in ParentBase (and accessible in Parent)
      // must be specified as field<Parent>(&Parent::accessor)
    template <typename Parent, typename ParentBase, typename ...Args> inline std::shared_ptr<readers::makers::Base<Parent>> field(void (ParentBase::*setter)(Args...))
    {
        using Setter = void (Parent::*)(Args...);
        return std::make_shared<readers::makers::Setter<Parent, Setter>>(setter);
    }

      // Field is an Object accessible via: Field& Parent::accessor()
    template <typename Parent, typename Field> inline std::shared_ptr<readers::makers::Base<Parent>> field(Field& (Parent::*accessor)(), data<Field>& aData)
    {
        return std::make_shared<readers::makers::Accessor<Parent, Field, decltype(accessor)>>(accessor, aData);
    }

      // Array of Objects
    template <typename Parent, typename Field> inline std::shared_ptr<readers::makers::Base<Parent>> field(std::vector<Field>& (Parent::*accessor)(), data<Field>& aData)
    {
        return std::make_shared<readers::makers::ArrayOfObjectsAccessor<Parent, Field, decltype(accessor)>>(accessor, aData);
    }

      // Array of values
    template <typename Parent, typename Field> inline std::shared_ptr<readers::makers::Base<Parent>> field(std::vector<Field>& (Parent::*accessor)())
    {
        using Storer = decltype(storers::type_detector<storers::ArrayElement<Field>>(std::declval<Field>()));
        return std::make_shared<readers::makers::ArrayOfValuesAccessor<Parent, Field, decltype(accessor), Storer>>(accessor);
    }

      // Array of values (with inheritance)
      // Access for Field of Parent, where
      //   Field is derived from std::vector<Element>
      //   Parent is derived from ParentBase, accessor declared in ParentBase (and accessible in Parent)
      // must be specified as field<Element, Parent>(&Parent::accessor)
    template <typename Element, typename Parent, typename ParentBase, typename Field> inline std::shared_ptr<readers::makers::Base<Parent>> field(Field& (ParentBase::*accessor)())
    {
        using Storer = decltype(storers::type_detector<storers::ArrayElement<Element>>(std::declval<Element>()));
        return std::make_shared<readers::makers::ArrayOfValuesAccessor<Parent, Element, decltype(accessor), Storer>>(accessor);
    }

      // Array of array of values
    template <typename Parent, typename Field> inline std::shared_ptr<readers::makers::Base<Parent>> field(std::vector<std::vector<Field>>& (Parent::*accessor)())
    {
        using Storer = decltype(storers::type_detector<storers::ArrayOfArrayElement<Field>>(std::declval<Field>()));
        return std::make_shared<readers::makers::ArrayOfArrayOfValuesAccessor<Parent, Field, decltype(accessor), Storer>>(accessor);
    }

      // Array of array of double with inheritance
    template <typename Parent, typename Field> inline std::shared_ptr<readers::makers::Base<Parent>> field(Field& (Parent::*accessor)())
    {
        using Storer = decltype(storers::type_detector<storers::ArrayOfArrayElementTarget<Field>>(std::declval<double>()));
        return std::make_shared<readers::makers::ArrayOfArrayOfValuesTargetAccessor<Parent, Field, decltype(accessor), Storer>>(accessor);
    }

      // Custom Storer (derived from storers::Base)
      // must be specified as field<Storer, Parent, Field>(&Parent::accessor)
    template <typename Storer, typename Parent, typename Field> inline std::shared_ptr<readers::makers::Base<Parent>> field(Field& (Parent::*accessor)())
    {
        return std::make_shared<readers::makers::GenericAccessor<Storer, Parent, Field>>(accessor);
    }

      // ----------------------------------------------------------------------

    template <typename Target> inline void import(std::string aSource, Target& aTarget, data<Target>& aData)
    {
        EventHandler handler{aTarget, aData};
        rapidjson::Reader reader;
        rapidjson::StringStream ss(aSource.c_str());
        reader.Parse(ss, handler);
        if (reader.HasParseError())
            throw std::runtime_error("json_importer failed at " + std::to_string(reader.GetErrorOffset()) + ": "
                                     +  GetParseError_En(reader.GetParseErrorCode()) + "\n"
                                     + aSource.substr(reader.GetErrorOffset(), 50));
    }

      // ----------------------------------------------------------------------

} // namespace json_importer

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
