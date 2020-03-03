#pragma once

#include <optional>
#include <iostream>

#include "acmacs-base/float.hh"
#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace acmacs::detail
{
    template <typename T> class field_optional_with_default
    {
     public:
        field_optional_with_default() = default;
        field_optional_with_default(const T aDefault) : mDefault{aDefault} {}
        field_optional_with_default(const field_optional_with_default&) = default;
        field_optional_with_default(field_optional_with_default&&) = default;
        field_optional_with_default& operator=(const field_optional_with_default& aOther)
            {
                if (aOther.present()) {
                      // assignment for T may actually mean modifying
                      // original value instead of completely
                      // overwriting it, it happens for Color. That is why we first assign to default, if necessary.
                    if (!mValue.has_value())
                        mValue = mDefault;
                    mValue = aOther.mValue;
                }
                return *this;
            }
        field_optional_with_default& operator=(field_optional_with_default&&) = default; // really move (unlike the above), to allow removing/inserting elements of vector<>
        field_optional_with_default& operator=(const T aValue) { mValue = aValue; return *this; }

        [[nodiscard]] bool operator==(const field_optional_with_default<T>& f) const { return mValue.value_or(mDefault) == f.mValue.value_or(f.mDefault); }

        bool is_default() const { return !mValue.has_value() || mValue.value() == mDefault; }
        bool not_default() const { return mValue.has_value() && mValue.value() != mDefault; }
        constexpr bool present() const { return mValue.has_value(); }

        constexpr operator const T() const { return mValue.value_or(mDefault); }
        constexpr const T operator*() const { return *this; }
        const std::decay_t<T>* operator->() const
            {
                if (present())
                    return &mValue.value();
                else
                    return &mDefault;
            }

        std::decay_t<T>& set() { if (!mValue.has_value()) mValue = mDefault; return mValue.value(); }

        std::string to_string() const
            {
                if (is_default())
                    return acmacs::to_string(mDefault) + "(default)";
                else
                    return acmacs::to_string(*mValue);
            }

     private:
        std::optional<std::decay_t<T>> mValue;
        T mDefault;             // non const to allow move assignment

    }; // class field_optional_with_default<>

    template <> inline bool field_optional_with_default<double>::operator==(const field_optional_with_default<double>& f) const
    {
        return ((mValue.has_value() && f.mValue.has_value() && float_equal(mValue.value(), f.mValue.value())) || (!mValue.has_value() && !f.mValue.has_value())) && float_equal(mDefault, f.mDefault);
    }

    template <> inline bool field_optional_with_default<double>::is_default() const
    {
        return !mValue.has_value() || float_equal(mValue.value(), mDefault);
    }

    template <> inline bool field_optional_with_default<double>::not_default() const
    {
        return mValue.has_value() && ! float_equal(mValue.value(), mDefault);
    }

} // namespace acmacs::detail

namespace acmacs
{
    template <typename T> inline std::string to_string(const acmacs::detail::field_optional_with_default<T>& field) { return field.to_string(); }

} // namespace acmacs

namespace acmacs::detail
{
    template <typename T> inline std::ostream& operator<<(std::ostream& s, const field_optional_with_default<T>& field) { return s << acmacs::to_string(field); }

} // namespace acmacs::detail

template <typename T> struct fmt::formatter<acmacs::detail::field_optional_with_default<T>> : fmt::formatter<acmacs::fmt_default_formatter> {
    template <typename FormatCtx> auto format(const acmacs::detail::field_optional_with_default<T>& val, FormatCtx& ctx) { return format_to(ctx.out(), "{}", *val); }
};


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
