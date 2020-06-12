#include "acmacs-base/string-substitute.hh"

// ----------------------------------------------------------------------

std::string acmacs::string::substitute_from_store(std::string_view pattern, const fmt::dynamic_format_arg_store<fmt::format_context>& store, if_no_substitution_found insf)
{
    return std::string{pattern};

} // acmacs::string::substitute_from_store

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
