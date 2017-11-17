#pragma once

#include <optional>
#include <iostream>
// #include <type_traits>

#include "acmacs-base/float.hh"

// ----------------------------------------------------------------------

namespace acmacs::internal
{
    template <typename T> class field_optional_with_default
    {
     public:
        inline field_optional_with_default() = default;
        inline field_optional_with_default(const T aDefault) : mDefault{aDefault} {}
        inline field_optional_with_default(const field_optional_with_default&) = default;
        inline field_optional_with_default& operator=(const field_optional_with_default& aOther)
            {
                if (aOther.present())
                    mValue = aOther.mValue;
                return *this;
            }
        inline field_optional_with_default& operator=(const T aValue) { mValue = aValue; return *this; }

        [[nodiscard]] inline bool operator==(const field_optional_with_default<T>& f) const { return mValue == f.mValue && mDefault == f.mDefault; }

        inline bool is_default() const { return !mValue.has_value() || mValue.value() == mDefault; }
        inline bool not_default() const { return mValue.has_value() && mValue.value() != mDefault; }
        constexpr inline bool present() const { return mValue.has_value(); }

        constexpr inline operator const T() const { return mValue.value_or(mDefault); }
        constexpr inline const T operator*() const { return *this; }
        inline const std::decay_t<T>* operator->() const
            {
                if (present())
                    return &mValue.value();
                else
                    return &mDefault;
            }

        inline std::decay_t<T>& set() { if (!mValue.has_value()) mValue = mDefault; return mValue.value(); }

     private:
        std::optional<std::decay_t<T>> mValue;
        const T mDefault;

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

} // namespace acmacs::internal

// ----------------------------------------------------------------------

template <typename T> inline std::ostream& operator<<(std::ostream& s, const acmacs::internal::field_optional_with_default<T>& field)
{
    s << *field;
    if (field.is_default())
        s << "(default)";
    return s;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
