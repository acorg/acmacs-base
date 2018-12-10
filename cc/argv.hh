#pragma once

#include <string_view>
#include <vector>

// ----------------------------------------------------------------------

namespace acmacs
{
    namespace argv
    {
        inline namespace v2
        {
            class argv;


            namespace detail
            {
                using cmd_line_iter = std::vector<const char*>::const_iterator;

                class option_base
                {
                  public:
                    option_base(argv& parent);
                    virtual ~option_base() = default;

                    virtual void add(cmd_line_iter& arg) = 0;
                    virtual void show(std::ostream& out) const = 0;

                    constexpr char short_name() const { return short_name_; }
                    constexpr std::string_view long_name() const { return long_name_; }

                  protected:
                    constexpr void use_arg(char short_name) { short_name_ = short_name; }
                    constexpr void use_arg(const char* long_name) { long_name_ = long_name; }
                    void show_names(std::ostream& out) const;

                  private:
                    char short_name_ = 0;
                    std::string_view long_name_;
                }; // class option_base

                inline std::ostream& operator<<(std::ostream& out, const option_base& opt) { opt.show(out); return out; }

            } // namespace detail

            // ----------------------------------------------------------------------

            template <typename T> class option : public detail::option_base
            {
              public:
                template <typename... Args> option(argv& parent, Args&&... args) : option_base(parent) { use_args(std::forward<Args>(args)...); }

                constexpr operator const T&() const { return value_; }

                void add(detail::cmd_line_iter& arg) override;
                void show(std::ostream& out) const override;

              protected:
                using detail::option_base::use_arg;

              private:
                T value_;

                template <typename Arg, typename... Args> void use_args(Arg&& arg, Args&&... args)
                {
                    use_arg(std::forward<Arg>(arg));
                    if constexpr (sizeof...(args) > 0)
                        use_args(std::forward<Args>(args)...);
                }
            }; // class option<T>

            // ----------------------------------------------------------------------

            class argv
            {
              protected:
                argv(int argc, const char* const argv[]);

              private:
                std::string_view prog_name_;
                std::vector<detail::option_base*> options_;
                std::vector<std::string_view> args_;
                std::vector<std::string> errors_;

                option<bool> show_help_{*this, 'h', "help"};

                void use(detail::cmd_line_iter& arg);
                detail::option_base* find(char short_name);
                detail::option_base* find(std::string_view long_name);

                void register_option(detail::option_base* opt);
                friend class detail::option_base;
                friend std::ostream& operator<<(std::ostream& out, const argv& args);

            }; // class argv

            // ----------------------------------------------------------------------

            inline detail::option_base::option_base(argv& parent) { parent.register_option(this); }

            // ----------------------------------------------------------------------

            template <> void option<bool>::add(detail::cmd_line_iter& /*arg*/)
            {
                value_ = true;
            }

            template <> void option<bool>::show(std::ostream& out) const
            {
                // if (value_)
                    show_names(out);
            }

        } // namespace v2
    }     // namespace argv
} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
