#include <algorithm>
#include <array>
#include <atomic>
#include <bzlib.h>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <condition_variable>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <getopt.h>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/mman.h>
#include <sys/types.h>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unistd.h>
#include <utility>
#include <variant>
#include <vector>

// ----------------------------------------------------------------------

#if __has_include(<filesystem>)
#include <filesystem>
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
#endif

#if __has_include(<experimental/iterator>)
#include <experimental/iterator>
#endif

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdeprecated-dynamic-exception-spec"
#pragma GCC diagnostic ignored "-Wdocumentation"
#endif
#include <lzma.h>
#pragma GCC diagnostic pop

#include <curl/curl.h>

// #include <openssl/md5.h>

// ----------------------------------------------------------------------

// #pragma GCC diagnostic push
// #ifdef __clang__
// #pragma GCC diagnostic ignored "-Wclass-varargs"
// #pragma GCC diagnostic ignored "-Wcovered-switch-default"
// #pragma GCC diagnostic ignored "-Wdeprecated"
// #pragma GCC diagnostic ignored "-Wdocumentation"
// #pragma GCC diagnostic ignored "-Wdocumentation-unknown-command"
// #pragma GCC diagnostic ignored "-Wexit-time-destructors"
// // #pragma GCC diagnostic ignored "-Wextra-semi"
// #pragma GCC diagnostic ignored "-Wfloat-equal"
// // #pragma GCC diagnostic ignored "-Wmissing-noreturn"
// #pragma GCC diagnostic ignored "-Wnested-anon-types"
// #pragma GCC diagnostic ignored "-Wold-style-cast"
// #pragma GCC diagnostic ignored "-Wrange-loop-analysis"
// #pragma GCC diagnostic ignored "-Wreserved-id-macro" // in Python.h
// #pragma GCC diagnostic ignored "-Wshadow"
// #pragma GCC diagnostic ignored "-Wshadow-field"
// // #pragma GCC diagnostic ignored "-Wsign-conversion"
// #pragma GCC diagnostic ignored "-Wundef"
// #pragma GCC diagnostic ignored "-Wundefined-reinterpret-cast"
// #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
// #endif
// #include <Python.h>
// #include <pybind11/pybind11.h>
// #include <pybind11/stl.h>
// #include <pybind11/stl_bind.h>
// #pragma GCC diagnostic pop

// ----------------------------------------------------------------------

// #include <cairo-pdf.h>

// ----------------------------------------------------------------------

// #include <uWS/uWS.h>

// ----------------------------------------------------------------------

// #include <bsoncxx/builder/basic/array.hpp>
// #include <bsoncxx/builder/basic/document.hpp>
// #include <bsoncxx/builder/stream/document.hpp>
// #include <bsoncxx/json.hpp>
// #include <bsoncxx/types.hpp>
// #include <bsoncxx/types/value.hpp>

// #include <mongocxx/client.hpp>
// #include <mongocxx/database.hpp>
// #include <mongocxx/exception/query_exception.hpp>
// #include <mongocxx/instance.hpp>
// #include <mongocxx/pool.hpp>

// #include <websocketpp/config/asio.hpp>
// #include <websocketpp/server.hpp>

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
