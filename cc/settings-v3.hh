#pragma once

#include <string_view>
#include <vector>
#include <memory>
#include <stdexcept>

#include "acmacs-base/rjson-v3.hh"

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
    enum class throw_if_nothing_applied { no, yes };
    enum class replace { no, yes, yes_or_set_at_bottom };

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
        // try to apply each in the names, stop upon finding and applying first
        void apply_first(const std::vector<std::string_view>& names, throw_if_nothing_applied tina);

        const rjson::v3::value& substitute(const rjson::v3::value& source) const;
        std::string substitute(std::string_view source) const;

        void setenv(std::string_view key, const rjson::v3::value& val);
        void setenv(std::string_view key, rjson::v3::value&& val, replace repl = replace::no);
        void setenv(std::string_view key, std::string_view val, replace repl = replace::no) { setenv(key, rjson::v3::detail::string(rjson::v3::detail::string::with_content, val), repl); }
        void setenv(std::string_view key, bool val, replace repl = replace::no) { setenv(key, rjson::v3::detail::boolean(val), repl); }

        const rjson::v3::value& getenv(std::string_view name) const;

        template <typename T> void getenv_copy_if_present(std::string_view key, T& target) const
        {
            static_assert(!std::is_same_v<std::decay_t<T>, std::string_view>);
            if (const auto& val = getenv(key); !val.is_null())
                target = val.to<std::decay_t<T>>();
        }

        std::string format_environment(std::string_view indent) const;

        class environment_push
        {
          public:
            environment_push(Data& data, bool push = true);
            ~environment_push();

          private:
            Data& data_;
            const bool push_;
        };

      protected:
        const detail::Environment& environment() const { return *environment_; }
        detail::Environment& environment() { return *environment_; }
        virtual bool apply_built_in(std::string_view name); // returns true if built-in command with that name found and applied
        const rjson::v3::value& get(std::string_view name, toplevel_only tlo) const;
        std::string format_toplevel() const;

        template <typename T> std::decay_t<T> getenv_or(std::string_view key, T&& a_default) const;

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
    extern template std::string_view Data::getenv_or(std::string_view, const std::string_view&) const;
    // extern template std::string Data::getenv_or(std::string_view, const std::string&) const;
    extern template size_t Data::getenv_or(std::string_view, size_t&&) const;
    extern template double Data::getenv_or(std::string_view, double&&) const;
    extern template bool Data::getenv_or(std::string_view, bool&&) const;

} // namespace acmacs::settings::v3

// ----------------------------------------------------------------------

template <> struct fmt::formatter<acmacs::settings::v3::replace> : fmt::formatter<acmacs::fmt_helper::default_formatter> {
    template <typename FormatCtx> auto format(acmacs::settings::v3::replace value, FormatCtx& ctx)
    {
        switch (value) {
            case acmacs::settings::v3::replace::no:
                format_to(ctx.out(), "no");
                break;
            case acmacs::settings::v3::replace::yes:
                format_to(ctx.out(), "yes");
                break;
            case acmacs::settings::v3::replace::yes_or_set_at_bottom:
                format_to(ctx.out(), "yes_or_set_at_bottom");
                break;
        }
        return ctx.out();
    }
};


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
