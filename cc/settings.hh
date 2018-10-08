#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include <functional>

#include "acmacs-base/sfinae.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/rjson.hh"

// ----------------------------------------------------------------------

namespace acmacs::settings
{
    inline namespace v1
    {
        class field_base;
        class object;
        template <typename T> class array_basic;
        template <typename T> class array;

          // --------------------------------------------------

        class not_set : public std::runtime_error
        {
         public:
            not_set(std::string path) : runtime_error("not set: " + path) {}
        };

          // --------------------------------------------------

        class base
        {
         public:
            base() = default;
            base(const base&) = default;
            base(base&&) = default;
            virtual ~base() = default;
            virtual void inject_default() = 0;
            virtual std::string path() const = 0;

         protected:
            virtual rjson::value& set() = 0;
            virtual const rjson::value& get() const = 0;

            friend class object;
            template <typename T> friend class array_basic;
            template <typename T> friend class array;
        };

          // --------------------------------------------------

        class container : public base
        {
         public:
            container() = default;
            container(const container&) = delete;
            container(container&&) = delete;
        };

          // --------------------------------------------------

        class object_base : public container
        {
         public:
            void inject_default() override;

         protected:
            void register_field(field_base* fld) { fields_.push_back(fld); }

         private:
            std::vector<field_base*> fields_;
            friend class field_base;
        };

          // --------------------------------------------------

        class field_base : public base
        {
         public:
            field_base(object_base& parent, const char* name) : parent_(parent), name_(name) { parent.register_field(this); }
            field_base(const field_base&) = delete;
            field_base(field_base&&) = delete;
            rjson::value& set() override { return parent_.set().set(name_); }
            const rjson::value& get() const override { return parent_.get().get(name_); }
            std::string path() const override { return parent_.path() + '.' + name_; }

         private:
            object_base& parent_;
            const char* name_;
        };

          // --------------------------------------------------

        class toplevel : public object_base
        {
         public:
            toplevel() = default;

            std::string path() const override { return {}; }
            std::string to_string() const { return rjson::to_string(value_); }
            std::string pretty() const { return rjson::pretty(value_); }

            void read_from_file(std::string filename) { value_ = rjson::parse_file(filename); }
            void update_from_file(std::string filename) { value_.update(rjson::parse_file(filename)); }
            void write_to_file(std::string filename) const { file::write(filename, static_cast<std::string_view>(rjson::pretty(value_))); }


         protected:
            rjson::value& set() override { if (value_.is_null()) value_ = rjson::object{}; return value_; }
            const rjson::value& get() const override { return value_; }

         private:
            rjson::value value_;
        };

          // --------------------------------------------------

        class object : public object_base
        {
         public:
            object(base& parent) : parent_(parent) {}
            std::string path() const override { return parent_.path(); }

         protected:
            rjson::value& set() override { auto& val = parent_.set(); if (val.is_null()) val = rjson::object{}; return val; }
            const rjson::value& get() const override { return parent_.get(); }

         private:
            base& parent_;

            friend inline std::ostream& operator<<(std::ostream& out, const object& obj) { return out << obj.get(); }
        };

          // --------------------------------------------------

        template <typename T> class array_basic : public container
        {
         public:
            array_basic(base& parent) : parent_(parent) {}
            std::string path() const override { return parent_.path(); }
              // void inject_default() override { std::for_each(content_.begin(), content_.end(), [](T& elt) { elt.inject_default(); }); }
            void inject_default() override {}
            bool empty() const { return parent_.get().empty(); }
            size_t size() const { return parent_.get().size(); }
            T operator[](size_t index) const { return get()[index]; }
            rjson::value& operator[](size_t index) { return set()[index]; }
            T append(const T& to_append) { return set().append(to_append); }
            void clear() { set().clear(); }

         protected:
            rjson::value& set() override { auto& val = parent_.set(); if (val.is_null()) val = rjson::array{}; return val; }
            const rjson::value& get() const override { return parent_.get(); }

         private:
            base& parent_;

            friend inline std::ostream& operator<<(std::ostream& out, const array_basic<T>& arr) { return out << arr.get(); }
        };

          // --------------------------------------------------

        template <typename T> class array_element : public base
        {
         public:
            array_element(rjson::value& val, std::string path) : value_(val), content_(*this), path_(path) {}
            array_element(const array_element& src) : value_(src.value_), content_(*this), path_(src.path_) {}
            array_element(array_element&& src) : value_(std::move(src.value_)), content_(*this), path_(std::move(src.path_)) {}
            array_element& operator=(const array_element& src) { value_ = src.value_; path_ = src.path_; }
            array_element& operator=(array_element&& src) { value_ = std::move(src.value_); path_ = std::move(src.path_); }
            std::string path() const override { return path_; }
            void inject_default() override { content_.inject_default(); }
            T& operator*() { return content_; }
            const T& operator*() const { return content_; }
            T* operator->() { return &content_; }
            const T* operator->() const { return &content_; }

         protected:
            rjson::value& set() override { return value_; }
            const rjson::value& get() const override { return value_; }

         private:
            std::reference_wrapper<rjson::value> value_;
            T content_;
            std::string path_;

            friend class object;
        };

        template <typename T> class array : public container
        {
         public:
            array(base& parent) : parent_(parent) {}
            std::string path() const override { return parent_.path(); }
            void inject_default() override {}
            bool empty() const { return parent_.get().empty(); }
            size_t size() const { return parent_.get().size(); }
            array_element<T> operator[](size_t index) const { return {get()[index], make_element_path(index)}; }
            array_element<T> operator[](size_t index) { return {set()[index], make_element_path(index)}; }
            array_element<T> append() { const size_t index = size(); array_element<T> res(set().append(rjson::object{}), make_element_path(index)); res.inject_default(); return res; }
            void clear() { set().clear(); }

         protected:
            rjson::value& set() override { auto& val = parent_.set(); if (val.is_null()) val = rjson::array{}; return val; }
            const rjson::value& get() const override { return parent_.get(); }

         private:
            base& parent_;

            std::string make_element_path(size_t index) { return path() + '[' + std::to_string(index) + ']'; }
            friend inline std::ostream& operator<<(std::ostream& out, const array_basic<T>& arr) { return out << arr.get(); }
        };

          // --------------------------------------------------

        template <typename T> class field : public field_base
        {
         public:
            field(object_base* parent, const char* name, T&& default_value) : field_base(*parent, name), default_(std::forward<T>(default_value)) { }
            field(object_base* parent, const char* name) : field_base(*parent, name) { }
            void inject_default() override;

            field& operator=(const T& source);
            operator T() const;

         private:
            std::optional<T> default_;
        };

        template <typename T> inline std::ostream& operator<<(std::ostream& out, const field<T>& fld) { return out << static_cast<T>(fld); }

          // --------------------------------------------------

          // T must be derived from object
        template <typename T> class field_object : public field_base
        {
         public:
            field_object(object_base* parent, const char* name) : field_base(*parent, name), content_(*this) {}
            void inject_default() override { content_.inject_default(); }
            T& operator*() { return content_; }
            const T& operator*() const { return content_; }
            T* operator->() { return &content_; }
            const T* operator->() const { return &content_; }

         private:
            T content_;
        };

        template <typename T> inline std::ostream& operator<<(std::ostream& out, const field_object<T>& fld) { return out << *fld; }

        // --------------------------------------------------

        template <typename T> class field_array : public field_base
        {
          public:
            field_array(object_base* parent, const char* name) : field_base(*parent, name), content_(*this) {}
            field_array(object_base* parent, const char* name, std::initializer_list<T> init) : field_base(*parent, name), content_(*this), default_{init} {}
            void inject_default() override;
            bool empty() const { return content_.empty(); }
            size_t size() const { return content_.size(); }
            T operator[](size_t index) const { return content_[index]; }
            void append(const T& to_append) { content_.append(to_append); }
            void append() { content_.append({}); }
            void clear() { content_.clear(); }

            array_basic<T>& operator*() { return content_; }
            const array_basic<T>& operator*() const { return content_; }

          private:
            array_basic<T> content_;
            std::vector<T> default_;
        };

        template <typename T> inline std::ostream& operator<<(std::ostream& out, const field_array<T>& fld) { return out << *fld; }

        // --------------------------------------------------

        template <typename T> class field_array_of : public field_base
        {
          public:
            field_array_of(object_base* parent, const char* name) : field_base(*parent, name), content_(*this) {}
            void inject_default() override {}
            bool empty() const { return content_.empty(); }
            size_t size() const { return content_.size(); }
            array_element<T> operator[](size_t index) { return content_[index]; }
            array_element<T> operator[](size_t index) const { return content_[index]; }
            array_element<T> append() { return content_.append(); }
            void clear() { content_.clear(); }

            array<T>& operator*() { return content_; }
            const array<T>& operator*() const { return content_; }

          private:
            array<T> content_;
        };


        // **********************************************************************
        // implementation
        // **********************************************************************

        inline void object_base::inject_default()
        {
            std::for_each(fields_.begin(), fields_.end(), [](field_base* fld) { fld->inject_default(); });
        }

        template <typename T> inline void field<T>::inject_default()
        {
            if (default_) {
                if (auto& val = set(); val.is_null())
                    val = *default_;
            }
        }

        template <typename T> inline field<T>& field<T>::operator=(const T& source)
        {
            set() = source;
            return *this;
        }

        template <typename T> inline field<T>::operator T() const
        {
            if (auto& val = get(); !val.is_null()) {
                if constexpr (std::is_same_v<std::decay_t<T>, bool>)
                    return val.get_bool();
                else
                    return val;
            }
            else if (default_)
                return *default_;
            else
                throw not_set(path());
        }

        template <typename T> inline void field_array<T>::inject_default()
        {
            if (empty() && !default_.empty()) {
                for (const T& val : default_)
                    append(val);
            }
        }

    } // namespace v1

} // namespace acmacs::settings

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
