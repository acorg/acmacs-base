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
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

namespace guile
{
    // struct Error : public std::runtime_error
    // {
    //     using std::runtime_error::runtime_error;
    // };


    template <typename Func> constexpr inline auto subr(Func func) { return reinterpret_cast<scm_t_subr>(func); }

    inline void load(std::string_view filename) { scm_c_primitive_load(filename.data()); }

    inline void load(const std::vector<std::string_view>& filenames)
    {
        for (const auto& filename : filenames)
            scm_c_primitive_load(filename.data());
    }

    inline std::string to_string(SCM src)
    {
        // if (!scm_is_string(src))
        //     throw Error{fmt::format("expected string but passed {}", "?")};
        std::string result(scm_c_string_length(src), '?');
        scm_to_locale_stringbuf(src, result.data(), result.size());
        return result;
    }

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

    template <typename Arg> inline SCM to_scm(Arg arg)
    {
        static_assert(std::is_same_v<std::decay<Arg>, int>, "no to_scm specialization defined");
        return scm_from_signed_integer(arg);
    }

    template <> inline SCM to_scm<double>(double arg) { return scm_from_double(arg); }
    inline SCM to_scm() { return VOID; }

    template <typename Value> inline Value from_scm(SCM arg)
    {
        static_assert(std::is_same_v<std::decay<Value>, int>, "no from_scm specialization defined");
        return scm_to_int(arg);
    }

    template <> inline double from_scm<double>(SCM arg) { return scm_to_double(arg); }
    template <> inline std::string from_scm<std::string>(SCM arg) { return to_string(arg); }

    template <typename Name, typename Func> requires acmacs::sfinae::is_string_v<Name> void define(Name&& name, Func func)
    {
        const char* name_ptr{nullptr};
        if constexpr (acmacs::sfinae::is_const_char_ptr_v<Name>)
            name_ptr = name;
        else
            name_ptr = name.data();
        scm_c_define_gsubr(name_ptr, FunctionTraits<Func>::ArgCount, 0, 0, guile::subr(func));
    }

    // template <typename Func> void define_scm(std::string_view name, Func func)
    // {
    //     int num_args{0};
    //     if constexpr (std::is_invocable_v<Func>)
    //         num_args = 0;
    //     if constexpr (std::is_invocable_v<Func, SCM>)
    //         num_args = 1;
    //     else if constexpr (std::is_invocable_v<Func, SCM, SCM>)
    //         num_args = 2;
    //     else
    //         static_assert(std::is_invocable_v<Func, void>, "guile::define: unsupported function");
    //     scm_c_define_gsubr(name.data(), num_args, 0, 0, guile::subr(func));
    // }

    // initialize guile and call passed function to define functions
    template <typename Func> inline void init(Func func)
    {
        scm_init_guile();
        func();
    }

    template <typename Func> inline void init(const std::vector<std::string_view>& filenames_to_load, Func func)
    {
        init(func);
        load(filenames_to_load);
    }

} // namespace guile

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
