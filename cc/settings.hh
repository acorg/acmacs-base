#pragma once

#include "acmacs-base/rjson-v3.hh"
#include "acmacs-base/to-json.hh"

// ----------------------------------------------------------------------

namespace acmacs::settings::inline v2
{
    class error : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    namespace detail
    {
        using env_data_t = acmacs::small_map_with_unique_keys_t<std::string, rjson::v3::value>;
    }

    class Settings
    {
      public:
        enum class toplevel_only { no, yes };
        using substitute_result_t = std::variant<const rjson::v3::value*, std::string>;

        Settings() = default;
        Settings(const std::vector<std::string_view>& filenames) { load(filenames); }
        virtual ~Settings() = default;

        // read settings from files, upon reading each file apply "init" in it (if found)
        void load(std::string_view filename);
        void load(const std::vector<std::string_view>& filenames);
        void reload();          // reset environament, re-load previously loaded files, apply "init" in loaded files

        // substitute vars in name, find name in environment or in data_ or in built-in and apply it
        // if name starts with ? do nothing
        // if name not found, throw
        virtual void apply(std::string_view name /* = "main" */);

        void setenv(std::string_view key, rjson::v3::value&& value) { environment_.add(key, std::move(value)); }
        void setenv(std::string_view key, std::string_view value) { setenv(key, rjson::v3::parse_string(value)); }
        void setenv_toplevel(std::string_view key, std::string_view value) { environment_.add_to_toplevel(key, value); } // rjson::v3::parse_string(fmt::format("\"{}\"", value))); }
        void setenv_toplevel(std::string_view key, bool value) { environment_.add_to_toplevel(key, value); } // rjson::v3::parse_string(fmt::format("{}", value))); }

        const rjson::v3::value& getenv_single_substitution(std::string_view key, toplevel_only a_toplevel_only = toplevel_only::no) const
        {
            return environment_.get(environment_.substitute_to_value(key).to<std::string_view>(), a_toplevel_only);
        }

        // returns const_null if not found
        const rjson::v3::value& getenv(std::string_view key, toplevel_only a_toplevel_only = toplevel_only::no) const;
        const rjson::v3::value& getenv(std::string_view key1, std::string_view key2, toplevel_only a_toplevel_only = toplevel_only::no) const; // value of key1 or key2
        std::string getenv_to_string(std::string_view key, toplevel_only a_toplevel_only = toplevel_only::no) const;
        const detail::env_data_t& getenv_toplevel() const { return environment_.toplevel(); }

        substitute_result_t substitute(const rjson::v3::value& source) const;
        const rjson::v3::value& substitute_to_value(const rjson::v3::value& source) const;
        bool substitute_to_bool(const rjson::v3::value& source) const;
        double substitute_to_double(const rjson::v3::value& source) const;
        std::string substitute_to_string(const rjson::v3::value& source) const;

        template <typename T> std::decay_t<T> getenv_or(std::string_view key, T&& a_default, toplevel_only a_toplevel_only = toplevel_only::no) const
        {
            if (const auto& val = getenv(key, a_toplevel_only); !val.is_null())
                return val.to<std::decay_t<T>>();
            else
                return std::move(a_default);
        }

        template <typename T> void getenv_copy_if_present(std::string_view key, T& target, toplevel_only a_toplevel_only = toplevel_only::no) const
        {
            static_assert(!std::is_same_v<std::decay_t<T>, std::string_view>);
            if (const auto& val = getenv(key, a_toplevel_only); !val.is_null())
                target = val.to<std::decay_t<T>>();
        }

        template <typename T1, typename T2> void getenv_extract_copy_if_present(std::string_view key, T2& target, toplevel_only a_toplevel_only = toplevel_only::no) const
        {
            static_assert(!std::is_same_v<std::decay_t<T2>, std::string_view>);
            if (const auto& val = getenv(key, a_toplevel_only); !val.is_null())
                target = T2{val.to<std::decay_t<T1>>()};
        }

        void printenv() const { environment_.print(); }

        template <typename... Args> void getenv_to_json(to_json::object& obj, std::string_view key, Args&&... keys)
        {
            if (const auto& val = getenv(key); !val.is_null())
                obj << to_json::key_val(key, to_json::raw(rjson::v3::format(val)));
            if constexpr (sizeof...(keys) > 0)
                getenv_to_json(obj, keys...);
        }

        template <typename ... Args> std::string getenv_to_json(Args&& ... keys)
        {
            to_json::object obj;
            getenv_to_json(obj, keys...);
            return obj.compact(to_json::json::embed_space::no);
        }

      protected:
        template <typename... Key> const rjson::v3::value& get(Key&&... keys) const
        {
            return loaded_data_.get(keys ...);
        }

        void apply(const rjson::v3::value& entry);
        virtual bool apply_built_in(std::string_view name); // returns true if built-in command with that name found and applied

        // substitute vars in name, find name in the top of data_ and apply it
        // do nothing if name starts with ? or if it is not found in top of data_
        virtual void apply_top(std::string_view name);

      private:
        class LoadedDataFiles
        {
          public:
            LoadedDataFiles() = default;

            void load(std::string_view filename);
            void reload(Settings& settings);
            const rjson::v3::value& get_top(std::string_view name) const { return file_data_.front()[name]; }

            template <typename... Key> const rjson::v3::value& get(Key&&... keys) const
            {
                for (const auto& per_file : file_data_) {
                    if (const auto& val = per_file.get(std::forward<Key>(keys)...); !val.is_null())
                        return val;
                }
                return rjson::v3::const_null;
            }

            void report() const
            {
                for (const auto& per_file : file_data_)
                    AD_DEBUG("{}", per_file);
            }

          private:
            std::vector<std::string> filenames_;
            std::vector<rjson::v3::value_read> file_data_;
        };

        class Environment
        {
          public:
            Environment() { push(); }

            const rjson::v3::value& get(std::string_view key, toplevel_only a_toplevel_only) const;
            void push() { env_data_.emplace_back(); }
            void pop() { env_data_.erase(std::prev(std::end(env_data_))); }
            size_t size() const { return env_data_.size(); }
            void add(std::string_view key, const rjson::v3::value& val) { env_data_.rbegin()->emplace_or_replace(std::string{key}, val); }
            void add(std::string_view key, rjson::v3::value&& val) { env_data_.rbegin()->emplace_or_replace(key, std::move(val)); }
            void add_to_toplevel(std::string_view key, rjson::v3::value&& val) { env_data_.begin()->emplace_or_replace(std::string{key}, std::move(val)); }
            void add_to_toplevel(std::string_view key, std::string_view value)  { env_data_.begin()->emplace_or_replace(std::string{key}, rjson::v3::detail::string{rjson::v3::detail::string::with_content, value}); }
            void add_to_toplevel(std::string_view key, bool value) { env_data_.begin()->emplace_or_replace(std::string{key}, rjson::v3::detail::boolean{value}); }
            void print() const;
            void print_key_value() const;

            std::string substitute_to_string(std::string_view source) const noexcept;
            const rjson::v3::value& substitute_to_value(std::string_view source) const noexcept;
            const detail::env_data_t& toplevel() const { return env_data_.back(); }

            // abort if source is empty
            // const rjson::v3::value* if substitution requested for the whole source
            // std::string if no substitution requested (returns source) or part of the source substituted
            substitute_result_t substitute(std::string_view source) const;

          private:
            std::vector<detail::env_data_t> env_data_;

        };

        LoadedDataFiles loaded_data_;
        mutable Environment environment_;
        mutable bool warn_if_set_used_{false};

        void push_and_apply(const rjson::v3::detail::object& entry);
        void apply_if();
        bool eval_condition(const rjson::v3::value& condition) const;
        bool eval_and(const rjson::v3::value& condition) const;
        bool eval_or(const rjson::v3::value& condition) const;
        bool eval_not(const rjson::v3::value& condition) const { return !eval_condition(condition); }
        bool eval_empty(const rjson::v3::value& condition, bool true_if_empty) const;
        bool eval_equal(const rjson::v3::value& condition) const;
        bool eval_not_equal(const rjson::v3::value& condition) const { return !eval_equal(condition); }

        friend class Subenvironment;

    }; // class Settings

} // namespace acmacs::settings::inline v2

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
