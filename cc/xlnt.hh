#pragma once

// https://github.com/tfussell/xlnt

#pragma GCC diagnostic push

// #pragma GCC diagnostic ignored "-Wunknown-pragmas"

#ifdef __clang__
// #pragma GCC diagnostic ignored "-Wdefaulted-function-deleted"
// #pragma GCC diagnostic ignored "-Wdocumentation"
#pragma GCC diagnostic ignored "-Wdocumentation-unknown-command"
// #pragma GCC diagnostic ignored "-Wfloat-equal"
// #pragma GCC diagnostic ignored "-Wnewline-eof"
#pragma GCC diagnostic ignored "-Wsuggest-destructor-override"
#endif

#ifdef __GNUG__
#endif

#include <xlnt/xlnt.hpp>

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
