#pragma once

#include <string_view>
#include <vector>
#include <memory>
#include <stdexcept>

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

    enum class toplevel_only { no, yes };

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
        virtual void apply(std::string_view name, toplevel_only tlo = toplevel_only::no);
        void apply(const rjson::v3::value& entry);

        const rjson::v3::value& substitute(const rjson::v3::value& source) const;
        std::string substitute(std::string_view source) const;

      protected:
        const detail::Environment& environment() const { return *environment_; }
        detail::Environment& environment() { return *environment_; }
        virtual bool apply_built_in(std::string_view name); // returns true if built-in command with that name found and applied
        const rjson::v3::value& get(std::string_view name, toplevel_only tlo) const;
        const rjson::v3::value& getenv(std::string_view name) const;
        std::string format_toplevel() const;
        void setenv(std::string_view key, const rjson::v3::value& val);
        void setenv(std::string_view key, rjson::v3::value&& val);
        void setenv(std::string_view key, std::string_view val);

        template <typename T> std::decay_t<T> getenv_or(std::string_view key, T&& a_default) const;

        class environment_push
        {
          public:
            environment_push(Data& data, bool push = true);
            ~environment_push();

          private:
            Data& data_;
            const bool push_;
        };

      private:
        std::unique_ptr<detail::LoadedDataFiles> loaded_data_;
        std::unique_ptr<detail::Environment> environment_;
        mutable bool warn_if_set_used_{false};

        // pushes values of object into environment, then calls apply(entry["N"]) unless entry["N"] == "set"
        void push_and_apply(const rjson::v3::detail::object& entry);

        void apply_if();
        void apply_for_each();

        bool eval_condition(const rjson::v3::value& condition) const;
        bool eval_and(const rjson::v3::value& condition) const;
        bool eval_or(const rjson::v3::value& condition) const;
        bool eval_not(const rjson::v3::value& condition) const { return !eval_condition(condition); }
        bool eval_empty(const rjson::v3::value& condition, bool true_if_empty) const;
        bool eval_equal(const rjson::v3::value& condition) const;
        bool eval_not_equal(const rjson::v3::value& condition) const { return !eval_equal(condition); }

    }; // class Data

    extern template std::string_view Data::getenv_or(std::string_view, std::string_view&&) const;

} // namespace acmacs::settings::v3

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
