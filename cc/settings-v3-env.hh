#pragma once

#include "acmacs-base/rjson-v3.hh"

// ----------------------------------------------------------------------

namespace acmacs::settings::v3
{
    class Data;

    namespace detail
    {
        class Environament
        {
          public:
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
