#pragma once

#include "acmacs-base/rjson-v3.hh"
#include "acmacs-base/flat-map.hh"

// ----------------------------------------------------------------------

namespace acmacs::settings::v3
{
    class Data;

    namespace detail
    {
        enum class toplevel_only { no, yes };

        using env_data_t = acmacs::small_map_with_unique_keys_t<std::string, rjson::v3::value>;

        class Environment
        {
          public:
            Environment() { push(); }

            const rjson::v3::value& get(std::string_view key, toplevel_only a_toplevel_only = toplevel_only::no) const;
            std::string substitute_to_string(std::string_view source) const;
            const rjson::v3::value& substitute(const rjson::v3::value& source) const;

            void add(std::string_view key, const rjson::v3::value& val) { env_data_.back().emplace_or_replace(std::string{key}, val); }
            void add(std::string_view key, rjson::v3::value&& val) { env_data_.back().emplace_or_replace(key, std::move(val)); }

            void push() { env_data_.emplace_back(); }
            void pop() { env_data_.pop_back(); }

            std::string format(std::string_view indent) const;

          private:
            std::vector<env_data_t> env_data_;

            const rjson::v3::value& substitute(std::string_view text, const rjson::v3::value& source) const;
        };

        // ----------------------------------------------------------------------

        class LoadedDataFiles
        {
          public:
            void load(std::string_view filename);
            void reload(Data& settings);

            const rjson::v3::value& top(std::string_view name) const { return file_data_.front()[name]; }

            template <typename... Key> const rjson::v3::value& get(Key&&... keys) const
            {
                for (const auto& per_file : file_data_) {
                    if (const auto& val = per_file.get(std::forward<Key>(keys)...); !val.is_null())
                        return val;
                }
                return rjson::v3::const_null;
            }

          private:
            std::vector<std::string> filenames_;
            std::vector<rjson::v3::value_read> file_data_;
        };

    } // namespace detail

} // namespace acmacs::settings::v3

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
