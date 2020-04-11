#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

#include "acmacs-base/string-join.hh"
#include "acmacs-base/string-from-chars.hh"
#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    namespace argv
    {
        inline namespace v2
        {
            class argv;
            using str = std::string_view;
            using str_array = std::vector<std::string_view>;

            struct desc : public std::string_view { using std::string_view::string_view; };
            struct arg_name : public std::string_view { using std::string_view::string_view; };
            enum mandatory { mandatory };
            template <typename T> struct dflt { dflt(T&& val) : value{std::move(val)} {} T value; explicit operator T() const { return value; } };
            dflt(const char*) -> dflt<str>;

            namespace detail
            {
                using cmd_line_iter = std::vector<const char*>::const_iterator;

                constexpr bool false_ = false;

                class option_base
                {
                  public:
                    option_base(argv& parent);
                    virtual ~option_base() = default;

                    virtual void add(cmd_line_iter& arg, cmd_line_iter last) = 0;
                    virtual void add(std::string_view arg) = 0;
                    std::string names() const noexcept;
                    constexpr bool has_name() const noexcept { return short_name_ || !long_name_.empty(); }
                    constexpr std::string_view description() const noexcept { return description_; }
                    virtual std::string get_default() const noexcept = 0;
                    virtual bool has_arg() const noexcept { return true; }
                    virtual bool has_value() const noexcept = 0;
                    virtual bool multiple_values() const noexcept = 0;
                    virtual bool is_bool() const noexcept = 0;
                    constexpr bool mandatory() const noexcept { return mandatory_; }

                    constexpr char short_name() const noexcept { return short_name_; }
                    constexpr std::string_view long_name() const noexcept { return long_name_; }
                    constexpr std::string_view arg_name() const noexcept { return arg_name_; }

                  protected:
                    constexpr void use_arg(char short_name) noexcept { short_name_ = short_name; }
                    constexpr void use_arg(const char* long_name) noexcept { long_name_ = long_name; }
                    constexpr void use_arg(desc&& description) noexcept { description_ = std::move(description); }
                    constexpr void use_arg(struct arg_name&& an) noexcept { arg_name_ = std::move(an); }
                    constexpr void use_arg(enum mandatory&&) noexcept { mandatory_ = true; }

                  private:
                    char short_name_ = 0;
                    std::string_view long_name_;
                    std::string_view description_;
                    std::string_view arg_name_{"ARG"};
                    bool mandatory_ = false;

                }; // class option_base

                template <typename T> T to_value(std::string_view source)
                {
                    if constexpr (std::is_same_v<T, bool>)
                        return std::stoi(std::string(source));
                    else if constexpr (std::is_arithmetic_v<T>)
                        return acmacs::string::from_chars<T>(source);
                    else
                        return source;
                }

                template <typename T> std::string to_string(const T& source) noexcept { return std::to_string(source); }
                // template <> std::string to_string(const std::string& source) { return '"' + source + '"'; }
                template <> std::string to_string(const str& source) noexcept { return fmt::format("\"{}\"", source); }
                template <> std::string to_string(const str_array& source) noexcept { return fmt::format("\"{}\"", string::join(std::string_view{"\" \""}, source)); }

                class invalid_option_value : public std::runtime_error { public: using std::runtime_error::runtime_error; };

            } // namespace detail

            // ----------------------------------------------------------------------

            template <typename T> class option : public detail::option_base
            {
              public:
                template <typename... Args> option(argv& parent, Args&&... args) : option_base(parent) { use_args(std::forward<Args>(args)...); }

                constexpr operator const T&() const { if (value_.has_value()) return *value_; else return default_; }
                constexpr const T& get() const { return static_cast<const T&>(*this); }
                constexpr const T* operator->() const { return &static_cast<const T&>(*this); }
                constexpr const T& operator*() const { return static_cast<const T&>(*this); }
                template <typename R> constexpr bool operator == (const R& rhs) const { return static_cast<const T&>(*this) == rhs; }
                template <typename R> constexpr bool operator != (const R& rhs) const { return !operator==(rhs); }
                std::string get_default() const noexcept override { return detail::to_string(default_); }
                bool has_arg() const noexcept override { return true; }
                bool has_value() const noexcept override { return value_.has_value(); }
                bool multiple_values() const noexcept override { return false; }

                bool is_bool() const noexcept override { if constexpr (std::is_same_v<T, bool>) return true; else return false; }

                bool empty() const noexcept { return get().empty(); }
                auto begin() const noexcept { return get().begin(); }
                auto end() const noexcept { return get().end(); }

                void add(detail::cmd_line_iter& arg, detail::cmd_line_iter last) override
                {
                    ++arg;
                    if (arg == last)
                        throw detail::invalid_option_value{"requires argument"};
                    add(*arg);
                }

                void add(std::string_view arg) override { value_ = detail::to_value<T>(arg); }

                template <typename Z = T, typename = std::enable_if_t<!std::is_same_v<Z, bool>>> explicit operator bool() const
                {
                    if constexpr (std::is_same_v<T, str>)
                        return has_value() && !get().empty();
                    else if constexpr (std::is_same_v<T, double>)
                        return has_value();
                    else
                        static_assert(std::is_same_v<T, void>, "operator bool not defined for this option type");
                }

              protected:
                using detail::option_base::use_arg;
                constexpr void use_arg(dflt<T>&& def) { default_ = static_cast<T>(def); }

              private:
                std::optional<T> value_;
                T default_;

                template <typename Arg, typename... Args> void use_args(Arg&& arg, Args&&... args)
                {
                    use_arg(std::forward<Arg>(arg));
                    if constexpr (sizeof...(args) > 0)
                        use_args(std::forward<Args>(args)...);
                }
            }; // class option<T>

            template <typename T> inline std::ostream& operator << (std::ostream& out, const option<T>& opt) { return out << opt.get(); }

            template <> void option<bool>::add(detail::cmd_line_iter& /*arg*/, detail::cmd_line_iter /*last*/) { value_ = true; }
            template <> bool option<bool>::has_arg() const noexcept { return false; }
            template <> constexpr option<bool>::operator const bool&() const { if (value_.has_value()) return *value_; return detail::false_; }
            template <> inline std::ostream& operator << (std::ostream& out, const option<bool>& opt) { return out << std::boolalpha << static_cast<const bool&>(opt); }

            template <> void option<str_array>::add(std::string_view arg) { if (!value_) value_ = str_array{arg}; else value_->push_back(arg); }
            template <> bool option<str_array>::multiple_values() const noexcept { return true; }

            template <typename T> using argument = option<T>;

            // ----------------------------------------------------------------------

            class description
            {
              public:
                description(argv& parent, std::string_view text);
                std::string_view text() const { return text_; }
              private:
                std::string_view text_;
            };

            // ----------------------------------------------------------------------

            class show_help : public std::exception {};
            class errors : public std::exception {};

            class argv
            {
             public:
                using errors_t = std::vector<std::string>;
                enum class on_error { exit, raise, return_false };
                  // returns true on success
                bool parse(int argc, const char* const argv[], on_error on_err = on_error::exit);

                constexpr auto argv0() const { return argv0_; }
                constexpr auto program_name() const { return prog_name_; }
                constexpr const errors_t& errors() const { return errors_; }
                void show_help(std::ostream& out) const;

                virtual ~argv() = default;

             protected:
                argv() = default;

              private:
                std::string_view argv0_;
                std::string_view prog_name_;
                std::vector<detail::option_base*> options_;
                std::vector<description*> descriptions_;
                std::vector<std::string_view> args_;
                errors_t errors_;

                option<bool> show_help_{*this, 'h', "help", desc{"show this help screen"}};

                void use(detail::cmd_line_iter& arg, detail::cmd_line_iter last);
                detail::option_base* find(char short_name);
                detail::option_base* find(std::string_view long_name);

                void register_option(detail::option_base* opt);
                void register_description(description* desc) { descriptions_.push_back(desc); }

                friend class detail::option_base;
                friend class description;
                friend std::ostream& operator<<(std::ostream& out, const argv& args);

            }; // class argv

            // ----------------------------------------------------------------------

            inline detail::option_base::option_base(argv& parent) { parent.register_option(this); }

            inline description::description(argv& parent, std::string_view text) : text_{text} { parent.register_description(this); }

            // ----------------------------------------------------------------------


        } // namespace v2
    }     // namespace argv
} // namespace acmacs

// ----------------------------------------------------------------------

// do not use template <typename T> struct fmt::formatter<acmacs::argv::option<T>>
// to avoid ambiguity between above and formatter for a collection (with begin() and end())

template <> struct fmt::formatter<acmacs::argv::option<acmacs::argv::str_array>> : fmt::formatter<acmacs::argv::str_array> {
    template <typename FormatCtx> auto format(const acmacs::argv::option<acmacs::argv::str_array>& opt, FormatCtx& ctx) { return fmt::formatter<acmacs::argv::str_array>::format(*opt, ctx); }
};

template <> struct fmt::formatter<acmacs::argv::option<size_t>> : fmt::formatter<size_t> {
    template <typename FormatCtx> auto format(const acmacs::argv::option<size_t>& opt, FormatCtx& ctx) { return fmt::formatter<size_t>::format(*opt, ctx); }
};

template <> struct fmt::formatter<acmacs::argv::option<int>> : fmt::formatter<int> {
    template <typename FormatCtx> auto format(const acmacs::argv::option<int>& opt, FormatCtx& ctx) { return fmt::formatter<int>::format(*opt, ctx); }
};

template <> struct fmt::formatter<acmacs::argv::option<double>> : fmt::formatter<double> {
    template <typename FormatCtx> auto format(const acmacs::argv::option<double>& opt, FormatCtx& ctx) { return fmt::formatter<double>::format(*opt, ctx); }
};

template <> struct fmt::formatter<acmacs::argv::option<bool>> : fmt::formatter<bool> {
    template <typename FormatCtx> auto format(const acmacs::argv::option<bool>& opt, FormatCtx& ctx) { return fmt::formatter<bool>::format(*opt, ctx); }
};


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
