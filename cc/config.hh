#pragma once

#ifdef __CHEERP_CLIENT__       // cheerp
#undef ACMACSD_FILESYSTEM
#define NO_EXCEPTIONS
#define union struct

// #define typeid typeid_simulation_for_cheerp
// struct s_typeid_simulation_for_cheerp
// {
//     const char* name() const { return "No-RTTI-in-chirp"; }
// };
// template <typename T> s_typeid_simulation_for_cheerp typeid_simulation_for_cheerp(const T&) { return s_typeid_simulation_for_cheerp{}; }

#define THROW(exc, cheerp_result) return (cheerp_result);
#define THROW_OR_VOID(exc) return;
#define THROW_OR_CERR(exc) std::cerr << "ERROR: " << (exc).what() << std::endl; return;

          // ----------------------------------------------------------------------
#else
#define ACMACSD_FILESYSTEM
#define THROW(exc, cheerp_result) throw (exc);
#define THROW_OR_VOID(exc) throw (exc);
#define THROW_OR_CERR(exc) throw (exc);
#endif

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
