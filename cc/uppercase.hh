#pragma once

#include <algorithm>
#include <cctype>

#include "acmacs-base/named-type.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    template <typename Tag> class uppercased : public named_string_t<Tag>
    {
      public:
        uppercased() = default;
        template <typename S> explicit uppercased(S&& source) : named_string_t<Tag>(std::forward<S>(source)) { std::transform(this->get().begin(), this->get().end(), this->get().begin(), ::toupper); }
        template <typename S> uppercased& operator=(S&& source) { const std::string_view src{source}; this->get().resize(src.size(), ' '); std::transform(src.begin(), src.end(), this->get().begin(), ::toupper); return *this; }
    };

    template <typename Tag> class lowercased : public named_string_t<Tag>
    {
      public:
        lowercased() = default;
        template <typename S> explicit lowercased(S&& source) : named_string_t<Tag>(std::forward<S>(source)) { std::transform(this->get().begin(), this->get().end(), this->get().begin(), ::tolower); }
        template <typename S> lowercased& operator=(S&& source) { const std::string_view src{source}; this->get().resize(src.size(), ' '); std::transform(src.begin(), src.end(), this->get().begin(), ::tolower); return *this; }
    };

    // ----------------------------------------------------------------------

    class uppercase : public uppercased<struct uppercase_tag>
    {
      public:
        uppercase() = default;
        template <typename S> uppercase(S&& source) : uppercased<struct uppercase_tag>(std::forward<S>(source)) {}
        operator std::string_view() const noexcept { return this->get(); }
    };

    class lowercase : public lowercased<struct lowercase_tag>
    {
      public:
        lowercase() = default;
        template <typename S> lowercase(S&& source) : lowercased<struct lowercase_tag>(std::forward<S>(source)) {}
        operator std::string_view() const noexcept { return this->get(); }
    };

} // namespace acmacs

// ----------------------------------------------------------------------

template <typename Tag> struct fmt::formatter<acmacs::uppercased<Tag>> : fmt::formatter<std::string> {
    template <typename FormatCtx> auto format(const acmacs::uppercased<Tag>& uc, FormatCtx& ctx) { return fmt::formatter<std::string>::format(uc.get(), ctx); }
};

template <typename Tag> struct fmt::formatter<acmacs::lowercased<Tag>> : fmt::formatter<std::string> {
    template <typename FormatCtx> auto format(const acmacs::lowercased<Tag>& uc, FormatCtx& ctx) { return fmt::formatter<std::string>::format(uc.get(), ctx); }
};

template <> struct fmt::formatter<acmacs::uppercase> : fmt::formatter<std::string> {
    template <typename FormatCtx> auto format(const acmacs::uppercase& uc, FormatCtx& ctx) { return fmt::formatter<std::string>::format(uc.get(), ctx); }
};

template <> struct fmt::formatter<acmacs::lowercase> : fmt::formatter<std::string> {
    template <typename FormatCtx> auto format(const acmacs::lowercase& uc, FormatCtx& ctx) { return fmt::formatter<std::string>::format(uc.get(), ctx); }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
