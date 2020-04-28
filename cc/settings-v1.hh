#pragma once

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wimplicit-int-float-conversion"
#endif

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <optional>
#include <functional>

#include "acmacs-base/debug.hh"
#include "acmacs-base/sfinae.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/rjson-v2.hh"
#include "acmacs-base/size.hh"
#include "acmacs-base/color.hh"
#include "acmacs-base/text-style.hh"

// ----------------------------------------------------------------------

namespace acmacs::settings
{
    namespace v1
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

            virtual rjson::v2::value& set() = 0;
            virtual const rjson::v2::value& get() const = 0;
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
            rjson::v2::value& set() override { return parent_.set().set(name_); }
            const rjson::v2::value& get() const override { return parent_.get().get(name_); }
            bool is_set() const { return !get().is_null(); }
            virtual void remove() { parent_.set().remove(name_); }
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
            std::string to_string() const { return rjson::v2::format(value_); }
            std::string pretty() const { return rjson::v2::pretty(value_); }

            void read_from_file(std::string_view filename) { value_ = rjson::v2::parse_file(filename, rjson::v2::remove_comments::no); }
            void update_from_file(std::string_view filename) { value_.update(rjson::v2::parse_file(filename, rjson::v2::remove_comments::no)); }
            void write_to_file(std::string_view filename, rjson::v2::emacs_indent emacs_indent = rjson::v2::emacs_indent::yes, const rjson::v2::PrettyHandler& pretty_handler = rjson::v2::PrettyHandler{}) const { file::write(filename, static_cast<std::string_view>(rjson::v2::pretty(value_, emacs_indent, pretty_handler))); }

            rjson::v2::value& set() override { if (value_.is_null()) value_ = rjson::v2::object{}; return value_; }
            const rjson::v2::value& get() const override { return value_; }

         private:
            rjson::v2::value value_;
        };

          // --------------------------------------------------

        class object : public object_base
        {
         public:
            object(base& parent) : parent_(parent) {}
            std::string path() const override { return parent_.path(); }
            std::string to_json() const { return rjson::v2::format(get()); }
            std::string pretty() const { return rjson::v2::pretty(get()); }

            rjson::v2::value& set() override { auto& val = parent_.set(); if (val.is_null()) val = rjson::v2::object{}; return val; }
            const rjson::v2::value& get() const override { return parent_.get(); }

         private:
            base& parent_;
        };

        inline std::ostream& operator<<(std::ostream& out, const object& obj) { return out << rjson::v2::format(obj.get()); }

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
            T operator[](size_t index) const { return get()[index].template to<T>(); }
            rjson::v2::value& operator[](size_t index) { return set()[index]; }
            T append(const T& to_append) { return set().append(to_append).template to<T>(); }
            void erase(size_t index) { set().remove(index); }
            void clear() { set().clear(); }
            void set(std::initializer_list<T> vals) { rjson::v2::value& stored = set(); stored.clear(); for (auto val : vals) stored.append(std::move(val)); }
            bool contains(const T& val) const { return bool(rjson::v2::find_index_if(get(), [&val](const auto& elt) { return elt == val; })); }
            template <typename Iter> void set(Iter first, Iter last) { rjson::v2::value& stored = set(); stored.clear(); for (; first != last; ++first) stored.append(*first); }
            template <typename F> void for_each(F func) const { rjson::v2::for_each(get(), [&func](const rjson::v2::value& val) -> void { func(val); }); }

            rjson::v2::value& set() override { auto& val = parent_.set(); if (val.is_null()) val = rjson::v2::array{}; return val; }
            const rjson::v2::value& get() const override { return parent_.get(); }

         private:
            base& parent_;
        };

        template <typename T> inline std::ostream& operator<<(std::ostream& out, const array_basic<T>& arr) { return out << rjson::v2::format(arr.get()); }

        // --------------------------------------------------

        template <typename T> class const_array_element : public base
        {
         public:
            const_array_element(const rjson::v2::value& val, std::string path) : value_(const_cast<rjson::v2::value&>(val)), content_(*this), path_(path) {}
            const_array_element(const const_array_element& src) : base(src), value_(src.value_), content_(*this), path_(src.path_) {}
            const_array_element(const_array_element&& src) : base(std::move(src)), value_(std::move(src.value_)), content_(*this), path_(std::move(src.path_)) {}
            const_array_element& operator=(const const_array_element& src) { value_ = src.value_; path_ = src.path_; }
            const_array_element& operator=(const_array_element&& src) { value_ = std::move(src.value_); path_ = std::move(src.path_); }
            std::string path() const override { return path_; }
            void inject_default() override {}
            const T& operator*() const { return content_; }
            const T* operator->() const { return &content_; }

            rjson::v2::value& set() override { return value_; }
            const rjson::v2::value& get() const override { return value_; }

         protected:
            T& content() { return content_; }

         private:
            std::reference_wrapper<rjson::v2::value> value_;
            T content_;
            std::string path_;

            friend class object;
        };

        template <typename T> class array_element : public const_array_element<T>
        {
         public:
            using const_array_element<T>::const_array_element;
            using const_array_element<T>::operator*;
            using const_array_element<T>::operator->;
            void inject_default() override { const_array_element<T>::content().inject_default(); }
            T& operator*() { return const_array_element<T>::content(); }
            T* operator->() { return &const_array_element<T>::content(); }
        };

        template <typename T> class array : public container
        {
         public:
            array(base& parent) : parent_(parent) {}
            std::string path() const override { return parent_.path(); }
            void inject_default() override {}
            bool empty() const { return parent_.get().empty(); }
            size_t size() const { return parent_.get().size(); }
            const_array_element<T> operator[](size_t index) const { return {get()[index], make_element_path(index)}; }
            array_element<T> operator[](size_t index) { return {set()[index], make_element_path(index)}; }
            array_element<T> append() { const size_t index = size(); array_element<T> res(set().append(rjson::v2::object{}), make_element_path(index)); res.inject_default(); return res; }
            void erase(size_t index) { set().remove(index); }
            void clear() { set().clear(); }
            template <typename F> std::optional<const_array_element<T>> find_if(F func) const;
            template <typename F> std::optional<array_element<T>> find_if(F func);
            template <typename F> std::optional<size_t> find_index_if(F func) const;
            template <typename F> void for_each(F func) const;
            template <typename F> void for_each(F func);

            rjson::v2::value& set() override { auto& val = parent_.set(); if (val.is_null()) val = rjson::v2::array{}; return val; }
            const rjson::v2::value& get() const override { return parent_.get(); }

         private:
            base& parent_;

            std::string make_element_path(size_t index) const { return path() + '[' + std::to_string(index) + ']'; }
        };

        template <typename T> inline std::ostream& operator<<(std::ostream& out, const array<T>& arr) { return out << rjson::v2::format(arr.get()); }

        // --------------------------------------------------

        template <typename T> class field : public field_base
        {
         public:
            field(object_base* parent, const char* name, const T& default_value) : field_base(*parent, name), default_(default_value) { }
            field(object_base* parent, const char* name, T&& default_value) : field_base(*parent, name), default_(std::move(default_value)) { }
            field(object_base* parent, const char* name, const char* default_value) : field_base(*parent, name), default_(T{default_value}) { }
            field(object_base* parent, const char* name) : field_base(*parent, name) { }
            void inject_default() override;

            field& operator=(const T& source);
            field& operator=(const field& source);
            bool is_set_or_has_default() const;
            T get_or(T&& a_default) const;
            T get_or(const T& a_default) const;
            operator T() const;
            T operator*() const { return *this; }
            bool empty() const;
            bool operator==(T rhs) const { return static_cast<T>(*this) == rhs; }
            bool operator!=(T rhs) const { return static_cast<T>(*this) != rhs; }
            bool operator<(T rhs) const { return static_cast<T>(*this) < rhs; }
            bool operator<=(T rhs) const { return static_cast<T>(*this) <= rhs; }
            bool operator>(T rhs) const { return static_cast<T>(*this) > rhs; }
            bool operator>=(T rhs) const { return static_cast<T>(*this) > rhs; }

         private:
            std::optional<T> default_;

              // to be specialised for complex types
            void assign(rjson::v2::value& to, const T& from) { to = from; }
            T extract(const rjson::v2::value& from) const { return from.to<T>(); }
        };

        template <typename T> inline std::ostream& operator<<(std::ostream& out, const field<T>& fld) { return out << fmt::format("{}", *fld); }

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
            void erase(size_t index) { content_.erase(index); }
            void clear() { content_.clear(); }
            using field_base::set;
            void set(std::initializer_list<T> vals) { content_.set(vals); }
            bool contains(const T& val) const { return content_.contains(val); }
            template <typename Iter> void set(Iter first, Iter last) { content_.set(first, last); }
            template <typename F> void for_each(F func) const { return content_.for_each(func); }
            template <typename F> void for_each(F func) { return content_.for_each(func); }

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
            const_array_element<T> operator[](size_t index) const { return content_[index]; }
            array_element<T> append() { return content_.append(); }
            void erase(size_t index) { content_.erase(index); }
            void clear() { content_.clear(); }
            template <typename F> std::optional<const_array_element<T>> find_if(F func) const { return content_.find_if(func); }
            template <typename F> std::optional<array_element<T>> find_if(F func) { return content_.find_if(func); }
            template <typename F> std::optional<size_t> find_index_if(F func) const { return content_.find_index_if(func); }
            template <typename F> void for_each(F func) const { return content_.for_each(func); }
            template <typename F> void for_each(F func) { return content_.for_each(func); }

            array<T>& operator*() { return content_; }
            const array<T>& operator*() const { return content_; }

          private:
            array<T> content_;
        };

        template <typename T> inline std::ostream& operator<<(std::ostream& out, const field_array_of<T>& fld) { return out << *fld; }

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
                    assign(val, *default_);
            }
        }

        template <typename T> inline field<T>& field<T>::operator=(const T& source)
        {
            assign(set(), source);
            return *this;
        }

        template <typename T> inline field<T>& field<T>::operator=(const field<T>& source)
        {
            assign(set(), static_cast<T>(source));
            return *this;
        }

        template <typename T> inline bool field<T>::is_set_or_has_default() const
        {
            return is_set() || default_;
        }

        template <typename T> inline field<T>::operator T() const
        {
            if (auto& val = get(); !val.is_null())
                return extract(val);
            else if (default_)
                return *default_;
            else
                throw not_set(path());
        }

        template <typename T> inline T field<T>::get_or(T&& a_default) const
        {
            try {
                return *this;
            }
            catch (not_set&) {
                return std::move(a_default);
            }
        }

        template <typename T> inline T field<T>::get_or(const T& a_default) const
        {
            try {
                return *this;
            }
            catch (not_set&) {
                return a_default;
            }
        }

        namespace detail { template <typename E> using has_empty_t = decltype(std::declval<E&>().empty()); }

        template <typename T> inline bool field<T>::empty() const
        {
            try {
                if constexpr (acmacs::sfinae::is_detected_v<detail::has_empty_t, T>)
                    return static_cast<T>(*this).empty();
                else
                    return false;
            }
            catch (not_set&) {
                return true;
            }
        }

        template <typename T> inline void field_array<T>::inject_default()
        {
            if (empty() && !default_.empty()) {
                for (const T& val : default_)
                    append(val);
            }
        }

        template <typename T> template <typename F> inline std::optional<size_t> array<T>::find_index_if(F func) const
        {
            if (const auto& rjval = get(); !rjval.is_null())
                return rjson::v2::find_index_if(rjval, [&func](const rjson::v2::value& val) -> bool { const_array_element<T> elt(val, ""); return func(*elt); });
            return {};
        }

        template <typename T> template <typename F> inline std::optional<const_array_element<T>> array<T>::find_if(F func) const
        {
            if (const auto index = find_index_if(func); index)
                return operator[](*index);
            return {};
        }

        template <typename T> template <typename F> inline std::optional<array_element<T>> array<T>::find_if(F func)
        {
            if (const auto index = find_index_if(func); index)
                return operator[](*index);
            return {};
        }

        template <typename T> template <typename F> inline void array<T>::for_each(F func) const
        {
            if (const auto& rjval = get(); !rjval.is_null())
                rjson::v2::for_each(rjval, [&func](const rjson::v2::value& val, size_t no) -> void {
                    const_array_element<T> elt(val, "");
                    func(*elt, no);
                });
        }

        template <typename T> template <typename F> inline void array<T>::for_each(F func)
        {
            if (!get().is_null()) {
                rjson::v2::for_each(set(), [&func](rjson::v2::value& val, size_t no) -> void {
                    array_element<T> elt(val, "");
                    func(*elt, no);
                });
            }
        }

          // ----------------------------------------------------------------------
          // specialisations
          // ----------------------------------------------------------------------

        template <> inline bool field<bool>::extract(const rjson::v2::value& from) const { return from.to<bool>(); }
        template <> inline std::string field<std::string>::extract(const rjson::v2::value& from) const { return from.to<std::string>(); }

        template <> inline void field<std::map<std::string, std::string>>::assign(rjson::v2::value& to, const std::map<std::string, std::string>& from)
        {
            to = rjson::v2::object{};
            for (const auto& [key, val] : from)
                to[key] = val;
        }
        template <> inline std::map<std::string, std::string> field<std::map<std::string, std::string>>::extract(const rjson::v2::value& from) const
        {
            std::map<std::string, std::string> result;
            rjson::v2::for_each(from, [&result](std::string_view field_name, const rjson::v2::value& item_value) { result.emplace(field_name, item_value.to<std::string>()); });
            return result;
        }

        template <> inline void field<Size>::assign(rjson::v2::value& to, const Size& from) { to = rjson::v2::array{from.width, from.height}; }
        template <> inline Size field<Size>::extract(const rjson::v2::value& from) const { return {from[0].to<double>(), from[1].to<double>()}; }

        template <> inline void field<Offset>::assign(rjson::v2::value& to, const Offset& from) { to = rjson::v2::array{from.x(), from.y()}; }
        template <> inline Offset field<Offset>::extract(const rjson::v2::value& from) const { return {from[0].to<double>(), from[1].to<double>()}; }

        template <> inline void field<Color>::assign(rjson::v2::value& to, const Color& from) { to = fmt::format("{}", from); }
        template <> inline Color field<Color>::extract(const rjson::v2::value& from) const { return Color(rjson::v2::to<std::string_view>(from)); }

        template <> inline void field<TextStyle>::assign(rjson::v2::value& to, const TextStyle& from)
        {
            using namespace std::string_view_literals;
            to = rjson::v2::object{{"family"sv, from.font_family}, {"slant"sv, fmt::format("{}", from.slant)}, {"weight"sv, fmt::format("{}", from.weight)}};
        }
        template <> inline TextStyle field<TextStyle>::extract(const rjson::v2::value& from) const
        {
            TextStyle style;
            assign_string_if_not_null(from["family"], style.font_family);
            assign_string_if_not_null(from["slant"], style.slant);
            assign_string_if_not_null(from["weight"], style.weight);
            return style;
        }

    } // namespace v1

} // namespace acmacs::settings

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
