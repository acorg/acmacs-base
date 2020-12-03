#pragma once

#include "acmacs-base/sfinae.hh"
#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wdeprecated-volatile"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wreserved-id-macro"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wextra-semi-stmt"
// #pragma GCC diagnostic ignored ""
#endif

#ifdef __GNUG__
#endif

#include <libguile.h>

namespace guile
{
    const inline auto VOID = SCM_UNSPECIFIED;
    const inline auto UNDEFINED = SCM_UNDEFINED;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

namespace guile
{
    // struct Error : public std::runtime_error
    // {
    //     using std::runtime_error::runtime_error;
    // };


    // ----------------------------------------------------------------------
    // init
    // ----------------------------------------------------------------------

    inline void load_script(std::string_view filename) { scm_c_primitive_load(filename.data()); }

    inline void load_script(const std::vector<std::string_view>& filenames)
    {
        for (const auto& filename : filenames)
            scm_c_primitive_load(filename.data());
    }

    // initialize guile, call passed functions to define functions or load scripts from files
    template <typename... Arg> inline void init(Arg&&... arg)
    {
        scm_init_guile();

        const auto process = []<typename Val>(Val&& val) -> void {
            if constexpr (std::is_invocable_v<Val>)
                val();
            else if constexpr (std::is_same_v<std::decay_t<Val>, std::string_view> || std::is_same_v<std::decay_t<Val>, std::vector<std::string_view>>)
                load_script(val);
            else
                static_assert(std::is_same_v<Val, void>);
        };
        (process(std::forward<Arg>(arg)), ...);
    }

    // ----------------------------------------------------------------------
    // define
    // ----------------------------------------------------------------------

    template <typename Func> constexpr inline auto subr(Func func) { return reinterpret_cast<scm_t_subr>(func); }

    // https://devblogs.microsoft.com/oldnewthing/20200713-00/?p=103978
    template <typename F> struct FunctionTraits;
    template <typename R, typename... Args> struct FunctionTraits<R (*)(Args...)>
    {
        using Pointer = R (*)(Args...);
        using RetType = R;
        using ArgTypes = std::tuple<Args...>;
        static constexpr std::size_t ArgCount = sizeof...(Args);
        // template <std::size_t N> using NthArg = std::tuple_element_t<N, ArgTypes>;
    };

    template <typename Name, typename Func> requires acmacs::sfinae::is_string_v<Name> void define(Name&& name, Func func)
    {
        const char* name_ptr{nullptr};
        if constexpr (acmacs::sfinae::is_const_char_ptr_v<Name>)
            name_ptr = name;
        else
            name_ptr = name.data();
        scm_c_define_gsubr(name_ptr, FunctionTraits<Func>::ArgCount, 0, 0, guile::subr(func));
    }

    // ----------------------------------------------------------------------
    // convert
    // ----------------------------------------------------------------------

    inline SCM symbol(const char* arg) { return scm_from_utf8_symbol(arg); }

    inline SCM to_scm() { return VOID; }
    inline SCM to_scm(double arg) { return scm_from_double(arg); }
    inline SCM to_scm(size_t arg) { return scm_from_size_t(arg); }
    inline SCM to_scm(ssize_t arg) { return scm_from_ssize_t(arg); }
    inline SCM to_scm(const std::string& arg) { return scm_from_locale_stringn(arg.data(), arg.size()); }

    template <typename Value> inline Value from_scm(SCM arg)
    {
        if constexpr (std::is_same_v<Value, std::string>) {
            std::string result(scm_c_string_length(arg), '?');
            scm_to_locale_stringbuf(arg, result.data(), result.size());
            return result;
        }
        else if constexpr (std::is_convertible_v<Value, size_t>)
            return scm_to_size_t(arg);
        else if constexpr (std::is_same_v<Value, double>) //  || std::is_same_v<Value, float>)
            return scm_to_double(arg);
        else
            static_assert(std::is_same_v<std::decay<Value>, int>, "no from_scm specialization defined");
    }

} // namespace guile

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
