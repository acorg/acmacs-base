#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <tuple>
#include <cstdio>
#include <cmath>
#include <sstream>
#include <iomanip>

// ----------------------------------------------------------------------

namespace acmacs
{
    // template <typename T> inline std::string to_string(T aSrc)
    // {
    //     return std::to_string(aSrc);
    // }

    inline std::string to_string(int src) { return std::to_string(src); }
    inline std::string to_string(unsigned src) { return std::to_string(src); }
    inline std::string to_string(long src) { return std::to_string(src); }
    inline std::string to_string(unsigned long src) { return std::to_string(src); }
    inline std::string to_string(long long src) { return std::to_string(src); }
    inline std::string to_string(unsigned long long src) { return std::to_string(src); }
    inline std::string to_string(float src) { return std::to_string(src); }
    inline std::string to_string(char src) { return std::string(1, src); }

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
    namespace internal
    {
        template <typename D> inline std::string to_string_double(D value, int precision, const char* format)
        {
            const auto num_digits_before_dot = static_cast<int>(std::log10(std::abs(value))) + 1;
            constexpr const size_t buffer_size = 100;
            char buffer[buffer_size + 1];
            const int written = std::snprintf(buffer, buffer_size, format, precision + num_digits_before_dot, value);
            if (written < 0 && static_cast<size_t>(written) >= buffer_size)
                throw std::runtime_error("acmacs::to_string(double) internal error");
            return {buffer, static_cast<size_t>(written)};
        }
    } // namespace internal
#pragma GCC diagnostic pop

    inline std::string to_string(double value, int precision = 32) { return internal::to_string_double(value, precision, "%.*g"); }
    inline std::string to_string(long double value, int precision = 64) { return internal::to_string_double(value, precision, "%.*Lg"); }

    inline std::string to_string(std::string src) { return src; }
    inline std::string to_string(std::string_view src) { return std::string(src); }
    inline std::string to_string(const char* src) { return src; }

    template <typename L, typename R> inline std::string to_string(const std::pair<L, R>& arg)
    {
        std::string result{'<'};
        result += to_string(arg.first);
        result += ", ";
        result += to_string(arg.second);
        result += '>';
        return result;
    }

    template <typename Iter, typename ... Args> inline std::string to_string(Iter first, Iter last, Args&& ... args)
    {
        std::string result{'['};
        while (first != last) {
            if (result.size() > 1)
                result += ", ";
            result += to_string(*first, std::forward<Args>(args) ...);
            ++first;
        }
        result += "]";
        return result;
    }

    template <typename Element, typename ... Args> inline std::string to_string(const std::vector<Element>& src, Args&& ... args)
    {
        return to_string(src.begin(), src.end(), std::forward<Args>(args) ...);
    }

      // ----------------------------------------------------------------------

    enum ShowBase { NotShowBase, ShowBase };
    enum OutputCase { Uppercase, Lowercase };

    template <typename T, typename = std::enable_if<std::is_integral<T>::value>> inline std::string to_hex_string(T aValue, enum ShowBase aShowBase, OutputCase aOutputCase = Uppercase)
    {
        std::ostringstream stream;
          // stream << (aShowBase == ShowBase ? std::showbase : std::noshowbase);
        if (aShowBase == ShowBase)
            stream << "0x";
        stream << std::setfill('0') << std::setw(sizeof(aValue)*2) << std::hex << std::noshowbase;
        stream << (aOutputCase == Uppercase ? std::uppercase : std::nouppercase);
        if constexpr (std::is_same_v<T, char> || std::is_same_v<T, unsigned char>) {
                stream << static_cast<unsigned>(aValue);
            }
        else {
            stream << aValue;
        }
        return stream.str();
    }

    template <typename T> inline std::string to_hex_string(const T* aPtr)
    {
        std::ostringstream stream;
        const void* value = reinterpret_cast<const void*>(aPtr);
        stream << value; // std::setfill('0') << std::setw(sizeof(value)*2) << std::hex << value;
        return stream.str();
    }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
