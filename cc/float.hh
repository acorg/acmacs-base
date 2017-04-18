#pragma once

#include <string>
#include <cmath>
#include <limits>
#include <type_traits>
#include <algorithm>
#include <stdexcept>

// ----------------------------------------------------------------------

// http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
template<typename T> typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
inline float_equal(T x, T y, int ulp=1)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::abs(x-y) < std::numeric_limits<T>::epsilon() * std::abs(x+y) * ulp
    // unless the result is subnormal
                           || std::abs(x-y) < std::numeric_limits<T>::min();
}

// ----------------------------------------------------------------------

template<typename T> typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
inline float_zero(T x, int ulp=1)
{
    return float_equal(x, T(0), ulp);
}

// ----------------------------------------------------------------------

inline std::string double_to_string(double value)
{
    constexpr const size_t buffer_size = 100;
    char buffer[buffer_size + 1];
    const int written = snprintf(buffer, buffer_size, "%.32g", value);
    if (written < 0 && static_cast<size_t>(written) >= buffer_size)
        throw std::runtime_error("double_to_string internal error");
    return {buffer, static_cast<size_t>(written)};
}

// ----------------------------------------------------------------------
