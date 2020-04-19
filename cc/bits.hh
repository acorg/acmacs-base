#pragma once

#include <bitset>

namespace acmacs::bits
{
    constexpr const auto num_bits{32};

    template <typename... Arg> inline std::bitset<num_bits> bitset_from_bool(bool val, Arg... args) noexcept
    {
        if constexpr (sizeof...(args) > 0)
            return (std::bitset<num_bits>{val} << sizeof...(args)) | bitset_from_bool(args...);
        else
            return std::bitset<num_bits>{val};
    }

    template <typename... Arg> inline size_t from_bool(Arg... args) noexcept
    {
        return bitset_from_bool(args ...).to_ulong();
    }

} // namespace acmacs::bits

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
