#pragma once

#include <string_view>
#include <vector>
#include <memory>
#include <stdexcept>

// #include "acmacs-base/rjson-v3.hh"
// #include "acmacs-base/to-json.hh"
// #include "acmacs-base/named-type.hh"

// ----------------------------------------------------------------------

namespace rjson::v3
{
    class value;
    namespace detail
    {
        class object;
    }
}

namespace acmacs::settings::v3
{
    class error : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    namespace detail
    {
        class Environment;
        class LoadedDataFiles;

    } // namespace detail

    class Data
    {
      public:
        Data();
        virtual ~Data();

        void load(std::string_view filename);
        void load(const std::vector<std::string_view>& filenames);
        void load_from_conf(const std::vector<std::string_view>& filenames); // load from ${ACMACSD_ROOT}/share/conf dir
        void set_defines(const std::vector<std::string_view>& defines);
        void reload(); // reset environament, re-load previously loaded files, apply "init" in loaded files

        // substitute vars in name, find name in environment or in data_ or in built-in and apply it
        // if name starts with ? do nothing
        // if name not found, throw
        virtual void apply(std::string_view name);
        void apply(const rjson::v3::value& entry);

      protected:
        virtual bool apply_built_in(std::string_view name); // returns true if built-in command with that name found and applied
        std::string substitute_to_string(std::string_view source) const;
        const rjson::v3::value& get(std::string_view name) const;

      private:
        std::unique_ptr<detail::LoadedDataFiles> loaded_data_;
        std::unique_ptr<detail::Environment> environment_;
        mutable bool warn_if_set_used_{false};

        // pushes values of object into environment, then calls apply(entry["N"]) unless entry["N"] == "set"
        void push_and_apply(const rjson::v3::detail::object& entry);

        void apply_if();
        void apply_for_each();

    };

} // namespace acmacs::settings::v3

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
