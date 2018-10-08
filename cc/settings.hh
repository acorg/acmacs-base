#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <optional>

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
            base(const base&) = delete;
            base(base&&) = delete;
            virtual ~base() = default;
            virtual void inject_default() = 0;
            virtual std::string path() const = 0;

         protected:
            virtual rjson::value& set() = 0;
            virtual const rjson::value& get() const = 0;

            friend class object;
        };

          // --------------------------------------------------

        class container : public base
        {
         protected:
            virtual void register_field(field_base* fld) = 0;

            friend class field_base;
        };

          // --------------------------------------------------

        class field_base : public base
        {
         public:
            field_base(container& parent, const char* name) : parent_(parent), name_(name) { parent.register_field(this); }
            rjson::value& set() override { return parent_.set().set(name_); }
            const rjson::value& get() const override { return parent_.get().get(name_); }
            std::string path() const override { return parent_.path() + '.' + name_; }

         private:
            container& parent_;
            const char* name_;
        };

          // --------------------------------------------------

        class object_base : public container
        {
         public:
            void inject_default() override { std::for_each(fields_.begin(), fields_.end(), [](field_base* fld) { fld->inject_default(); }); }

         protected:
            void register_field(field_base* fld) override { fields_.push_back(fld); }

         private:
            std::vector<field_base*> fields_;
        };

          // --------------------------------------------------

        class toplevel : public object_base
        {
         public:
            toplevel() = default;

            std::string path() const override { return {}; }
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
        };

          // --------------------------------------------------

        template <typename T> class field : public field_base
        {
         public:
            field(container* parent, const char* name, T&& default_value) : field_base(*parent, name), default_(std::forward<T>(default_value)) { }
            field(container* parent, const char* name) : field_base(*parent, name) { }
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
            field_object(container* parent, const char* name) : field_base(*parent, name), content_(*this) {}
            void inject_default() override { content_.inject_default(); }
            T* operator->() { return &content_; }
            const T* operator->() const { return &content_; }

         private:
            T content_;
        };

// **********************************************************************
// implementation
// **********************************************************************

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

    } // namespace v1

} // namespace acmacs::settings

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
