// No pragma once!!

// for clang and gcc
#pragma GCC diagnostic ignored "-Wunused-parameter"

// ----------------------------------------------------------------------
// just for clang
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wcomma"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wcovered-switch-default"
#pragma GCC diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#pragma GCC diagnostic ignored "-Wdocumentation"
#pragma GCC diagnostic ignored "-Wdocumentation-unknown-command"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wextra-semi"
#pragma GCC diagnostic ignored "-Wextra-semi-stmt"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wredundant-parens"
#pragma GCC diagnostic ignored "-Wreserved-id-macro"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wshadow-field"
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wundefined-func-template"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

// #pragma GCC diagnostic ignored "-Watomic-implicit-seq-cst"  // boost@1.60
// #pragma GCC diagnostic ignored "-Wextra-semi-stmt"  // boost@1.60

// ----------------------------------------------------------------------

#else // g++-7

#pragma GCC diagnostic ignored "-Wimplicit-fallthrough" // boost/asio

#endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
