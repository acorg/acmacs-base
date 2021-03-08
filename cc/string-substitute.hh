#pragma once

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace acmacs::string
{
    enum class if_no_substitution_found { leave_as_is, empty };

    std::string substitute_from_store(std::string_view pattern, const fmt::dynamic_format_arg_store<fmt::format_context>& store, if_no_substitution_found insf = if_no_substitution_found::leave_as_is);

    template <typename ... Args> std::string substitute(std::string_view pattern, Args&& ... args)
    {
        fmt::dynamic_format_arg_store<fmt::format_context> store;
        (store.push_back(fmt::arg(args.first, args.second)), ...);
        return substitute_from_store(pattern, store, if_no_substitution_found::leave_as_is);
    }

    std::vector<std::string_view> split_for_formatting(std::string_view source);

}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
