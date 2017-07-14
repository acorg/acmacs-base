#pragma once

#include <chrono>
#include <ctime>

// ----------------------------------------------------------------------

template <typename ConvertTime> inline std::string time_format(std::time_t aTime, const char* aFormat, ConvertTime convert_time)
{
    char buf[100];
    std::strftime(buf, sizeof buf, aFormat, convert_time(&aTime));
    return buf;
}

// ----------------------------------------------------------------------

inline std::string time_format(const std::chrono::system_clock::time_point& aTime, const char* aFormat)
{
    return time_format(std::chrono::system_clock::to_time_t(aTime), aFormat, &std::localtime);
}

inline std::string time_format_gm(const std::chrono::system_clock::time_point& aTime, const char* aFormat)
{
    return time_format(std::chrono::system_clock::to_time_t(aTime), aFormat, &std::gmtime);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
