#pragma once

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdocumentation"
#pragma GCC diagnostic ignored "-Wdocumentation-unknown-command"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wshadow"
#endif

// #include <range/v3/core.hpp>
// #include <range/v3/algorithm.hpp>

#include <range/v3/all.hpp>

#pragma GCC diagnostic pop

// https://github.com/JeffGarland/range_by_example
// https://www.walletfox.com/course/examples_range_v3.php

// ----------------------------------------------------------------------

template <typename T> inline auto range_from_0_to(T end)
{
    return ranges::views::iota(T{0}, end);
}

template <typename T> inline auto range_from_0_to_including(T end)
{
    return ranges::views::closed_iota(T{0}, end);
}

template <typename T> inline auto range_from_1_to_including(T end)
{
    return ranges::views::closed_iota(T{1}, end);
}

// ----------------------------------------------------------------------

template <typename T> inline void sort_unique(T& coll)
{
    coll |= ranges::actions::sort | ranges::actions::unique;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
