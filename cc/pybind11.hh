#pragma once

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
// Python.h 3.9 2020-12-06
#pragma GCC diagnostic ignored "-Wreserved-id-macro"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wcast-align"

// pybind11 2.6.1 2020-12-06
#pragma GCC diagnostic ignored "-Wshadow-field-in-constructor"
#pragma GCC diagnostic ignored "-Wdocumentation-unknown-command"
#pragma GCC diagnostic ignored "-Wextra-semi"
#pragma GCC diagnostic ignored "-Wshadow-uncaptured-local"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wcovered-switch-default"
#pragma GCC diagnostic ignored "-Wshadow-field"
#pragma GCC diagnostic ignored "-Wdocumentation"
#pragma GCC diagnostic ignored "-Wundefined-reinterpret-cast"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/embed.h>
#include <pybind11/functional.h>

#pragma GCC diagnostic pop

namespace py = pybind11;

// ----------------------------------------------------------------------

template <> struct fmt::formatter<py::object> : fmt::formatter<acmacs::fmt_helper::default_formatter> {
    template <typename FormatCtx> auto format(const py::object& value, FormatCtx& ctx)
    {
        return format_to(ctx.out(), "{}", py::repr(value).cast<std::string>());
    }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
