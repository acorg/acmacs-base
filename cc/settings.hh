#pragma once

#include "acmacs-base/fmt.hh"
#include "acmacs-base/rjson.hh"
#include "acmacs-base/flat-map.hh"

// ----------------------------------------------------------------------

namespace acmacs::settings
{
    inline namespace v2
    {
        class error : public std::runtime_error { using std::runtime_error::runtime_error; };

        class Settings
        {
          public:
            Settings() = default;
            Settings(const std::vector<std::string_view>& filenames) { load(filenames); }
            virtual ~Settings() = default;
            void load(const std::vector<std::string_view>& filenames);

            virtual void apply(std::string_view name = "main") const;
            virtual bool apply_built_in(std::string_view name) const; // returns true if built-in command with that name found and applied

          protected:
            template <typename ... Key> const rjson::value& get(Key&& ... keys) const;
            void apply(const rjson::value& entry) const;

          private:
            class Environment
            {
              public:
                Environment() { push(); }

                const rjson::value& get(std::string_view key) const;
                void push() { data_.emplace_back(); }
                void pop() { data_.erase(std::prev(std::end(data_))); }
                size_t size() const { return data_.size(); }
                void add(const std::string& key, const rjson::value& val) { data_.rbegin()->emplace_or_replace(key, val); }
                void print() const;

              private:
                std::vector<acmacs::flat_map_t<std::string, rjson::value>> data_;
            };

            std::vector<rjson::value> data_;
            mutable Environment environment_;

            void push_and_apply(const rjson::object& entry) const;

            friend class Subenvironment;
        };
    }
}

// ----------------------------------------------------------------------

template <typename... Key> const rjson::value& acmacs::settings::v2::Settings::get(Key&&... keys) const
{
    for (const auto& root : data_) {
        if (const auto& val = root.get(std::forward<Key>(keys)...); !val.is_const_null())
            return val;
    }
    return rjson::ConstNull;

} // acmacs::settings::v2::Settings::get

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
