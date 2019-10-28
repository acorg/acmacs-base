#pragma once

#include "acmacs-base/fmt.hh"
#include "acmacs-base/debug.hh"
#include "acmacs-base/rjson.hh"
#include "acmacs-base/flat-map.hh"

// ----------------------------------------------------------------------

namespace acmacs::settings::inline v2
{
    class error : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    class Settings
    {
      public:
        Settings() = default;
        Settings(const std::vector<std::string_view>& filenames) { load(filenames); }
        virtual ~Settings() = default;

        // read settings from files, upon reading each file apply "init" in it (if found)
        void load(const std::vector<std::string_view>& filenames)
        {
            for (const auto& filename : filenames)
                load(filename);
        }
        void load(std::string_view filename);

        // substitute vars in name, find name in environment or in data_ or in built-in and apply it
        // if name starts with ? do nothing
        // if name not found, throw
        virtual void apply(std::string_view name = "main", verbose verb = verbose::yes);
        void apply(const char* name, verbose verb = verbose::yes) { apply(std::string_view{name}, verb); }

        // substitute vars in name, find name in the top of data_ and apply it
        // do nothing if name starts with ? or if it is not found in top of data_
        virtual void apply_top(std::string_view name);

        virtual bool apply_built_in(std::string_view name); // returns true if built-in command with that name found and applied

        void setenv_from_string(std::string_view key, std::string_view value);
        template <typename T> void setenv(std::string_view key, T&& value) { setenv(key, rjson::value{std::forward<T>(value)}); }
        template <typename T> void setenv_toplevel(std::string_view key, T&& value) { setenv_toplevel(key, rjson::value{std::forward<T>(value)}); }

        const rjson::value& getenv(std::string_view key) const { return environment_.get(environment_.substitute(key).to<std::string>()); }
        template <typename T> T getenv(std::string_view key, T&& a_default) const
        {
            if (const auto& val = environment_.get(environment_.substitute(key).to<std::string>()); val.is_string()) {
                auto orig = val.to<std::string>();
                for (size_t num_subst = 0; num_subst < 10; ++num_subst) {
                    const auto substituted = environment_.substitute(std::string_view{orig});
                    if (substituted.is_string() && orig != substituted.to<std::string>())
                        orig = substituted.to<std::string>();
                    else if (substituted.is_const_null()) // substitutions lead to const-null
                        return std::move(a_default);
                    else
                        return substituted.to<T>();
                }
                throw error(fmt::format("Settings::getenv: too many substitutions in {}", rjson::to_string(val)));
            }
            else if (!val.is_const_null())
                return val.to<T>();
            else
                return std::move(a_default);
        }

        std::string getenv(std::string_view key, const char* a_default) const { return getenv(key, std::string{a_default}); }

        void printenv() const { environment_.print(); }

      protected:
        template <typename... Key> const rjson::value& get(Key&&... keys) const;
        void apply(const rjson::value& entry, verbose verb);

      private:
        class Environment
        {
          public:
            Environment() { push(); }

            const rjson::value& get(std::string_view key) const;
            void push() { data_.emplace_back(); }
            void pop() { data_.erase(std::prev(std::end(data_))); }
            size_t size() const { return data_.size(); }
            void add(std::string_view key, const rjson::value& val) { data_.rbegin()->emplace_or_replace(std::string{key}, val); }
            void add(std::string_view key, rjson::value&& val) { data_.rbegin()->emplace_or_replace(std::string{key}, std::move(val)); }
            void add_to_toplevel(std::string_view key, const rjson::value& val) { data_.begin()->emplace_or_replace(std::string{key}, val); }
            void add_to_toplevel(std::string_view key, rjson::value&& val) { data_.begin()->emplace_or_replace(std::string{key}, std::move(val)); }
            void print() const;
            void print_key_value() const;

            rjson::value substitute(std::string_view source) const;

          private:
            std::vector<acmacs::flat_map_t<std::string, rjson::value>> data_;

            rjson::value substitute(const rjson::value& source) const;
        };

        std::vector<rjson::value> data_;
        mutable Environment environment_;
        mutable bool warn_if_set_used_{false};

        void push_and_apply(const rjson::object& entry, verbose verb);
        void apply_if();
        bool eval_condition(const rjson::value& condition) const;
        bool eval_and(const rjson::value& condition) const;
        bool eval_or(const rjson::value& condition) const;
        bool eval_not(const rjson::value& condition) const;
        bool eval_empty(const rjson::value& condition, bool true_if_empty) const;

        friend class Subenvironment;
    }; // class Settings
} // namespace acmacs::settings::inline v2

// ----------------------------------------------------------------------

namespace acmacs::settings::inline v2
{
    template <typename... Key> const rjson::value& Settings::get(Key && ... keys) const
    {
        for (auto it = data_.rbegin(); it != data_.rend(); ++it) {
            if (const auto& val = it->get(std::forward<Key>(keys)...); !val.is_const_null())
                return val;
        }
        return rjson::ConstNull;

    } // acmacs::settings::v2::Settings::get

    template <> inline void Settings::setenv(std::string_view key, const rjson::value& value) { environment_.add(key, value); }
    template <> inline void Settings::setenv(std::string_view key, rjson::value && value) { environment_.add(key, std::move(value)); }
    template <> inline void Settings::setenv_toplevel(std::string_view key, const rjson::value& value) { environment_.add_to_toplevel(key, value); }
    template <> inline void Settings::setenv_toplevel(std::string_view key, rjson::value && value) { environment_.add_to_toplevel(key, std::move(value)); }

} // namespace acmacs::settings::inline v2

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
