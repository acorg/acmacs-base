#pragma once

// ----------------------------------------------------------------------
#ifdef _OPENMP

#include <omp.h>

// ----------------------------------------------------------------------
#else

inline int omp_get_thread_num() { return 0; }
inline int omp_get_num_threads() { return 1; }
inline int omp_get_max_threads() { return 1; }

#endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
