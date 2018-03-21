#pragma once

// ----------------------------------------------------------------------
#ifdef _OPENMP

#include <omp.h>

// ----------------------------------------------------------------------
#else

constexpr int omp_get_thread_num() { return 0; }
constexpr int omp_get_num_threads() { return 1; }
constexpr int omp_get_max_threads() { return 1; }

#endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
