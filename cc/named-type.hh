#pragma once

#include <variant>

#include "acmacs-base/string.hh"
#include "acmacs-base/fmt.hh"
#include "acmacs-base/to-string.hh"
#include "acmacs-base/float.hh"
#include "acmacs-base/range.hh"

// Simplified version of https://github.com/joboccara/NamedType
// Also see https://github.com/Enhex/phantom_type

// ----------------------------------------------------------------------

namespace acmacs
{
    template <typename T, typename Tag> class named_t
    {
      public:
        using value_type = T;

        explicit constexpr named_t() = default;
        template <typename T2, typename = std::enable_if_t<std::is_constructible_v<T, T2>>> explicit constexpr named_t(T2&& value) : value_(std::forward<T2>(value)) {}

        // template <typename T2, typename = std::enable_if_t<std::is_same_v<T, size_t>>>
        //     explicit constexpr named_t(T2 value) : value_(static_cast<T>(value)) {}

        // template <typename TT> using is_not_reference_t = typename std::enable_if<!std::is_reference<TT>::value, void>::type;
        // explicit constexpr named_t(const T& value) : value_(value) {}
        // explicit constexpr named_t(T&& value) : value_(std::move(value)) {}
        // template <typename T2 = T, typename = is_not_reference_t<T2>> explicit constexpr named_t(T2&& value) : value_(std::move(value)) {}

        constexpr T& get() noexcept { return value_; }
        constexpr const T& get() const noexcept { return value_; }
        constexpr T& operator*() noexcept { return value_; }
        constexpr const T& operator*() const noexcept { return value_; }
        constexpr const T* operator->() const noexcept { return &value_; }
        explicit constexpr operator T&() noexcept { return value_; }
        explicit constexpr operator const T&() const noexcept { return value_; }

      private:
        T value_;
    };

    template <typename T, typename Tag> constexpr bool operator==(const named_t<T, Tag>& lhs, const named_t<T, Tag>& rhs) noexcept { return lhs.get() == rhs.get(); }
    template <typename T, typename Tag> constexpr bool operator!=(const named_t<T, Tag>& lhs, const named_t<T, Tag>& rhs) noexcept { return !operator==(lhs, rhs); }

    template <typename T, typename Tag> inline std::string to_string(const named_t<T, Tag>& nt) { return acmacs::to_string(nt.get()); }

    // ----------------------------------------------------------------------

    template <typename Number, typename Tag> class named_number_t : public named_t<Number, Tag>
    {
      public:
        template <typename TT, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<TT>>, char>>
            explicit constexpr named_number_t(TT&& value) : named_t<Number, Tag>(static_cast<Number>(std::forward<TT>(value))) {}

        constexpr auto& operator++()
        {
            ++this->get();
            return *this;
        }
        constexpr auto& operator--()
        {
            --this->get();
            return *this;
        }
    };

    template <typename Number, typename Tag> constexpr bool operator<(named_number_t<Number, Tag> lhs, named_number_t<Number, Tag> rhs) noexcept { return lhs.get() < rhs.get(); }
    template <typename Number, typename Tag> constexpr bool operator<=(named_number_t<Number, Tag> lhs, named_number_t<Number, Tag> rhs) noexcept { return lhs.get() <= rhs.get(); }
    template <typename Number, typename Tag> constexpr bool operator>(named_number_t<Number, Tag> lhs, named_number_t<Number, Tag> rhs) noexcept { return lhs.get() > rhs.get(); }
    template <typename Number, typename Tag> constexpr bool operator>=(named_number_t<Number, Tag> lhs, named_number_t<Number, Tag> rhs) noexcept { return lhs.get() >= rhs.get(); }

    template <typename Number, typename Tag> constexpr named_number_t<Number, Tag> operator+(named_number_t<Number, Tag> lhs, named_number_t<Number, Tag> rhs) noexcept { return named_number_t<Number, Tag>{lhs.get() + rhs.get()}; }
    template <typename Number, typename Tag> constexpr named_number_t<Number, Tag> operator+(named_number_t<Number, Tag> lhs, Number rhs) noexcept { return named_number_t<Number, Tag>{lhs.get() + rhs}; }
    template <typename Number, typename Tag> constexpr named_number_t<Number, Tag> operator-(named_number_t<Number, Tag> lhs, named_number_t<Number, Tag> rhs) noexcept { return named_number_t<Number, Tag>{lhs.get() - rhs.get()}; }
    template <typename Number, typename Tag> constexpr named_number_t<Number, Tag> operator-(named_number_t<Number, Tag> lhs, Number rhs) noexcept { return named_number_t<Number, Tag>{lhs.get() - rhs}; }
    template <typename Number, typename Tag> constexpr named_number_t<Number, Tag> operator*(named_number_t<Number, Tag> lhs, named_number_t<Number, Tag> rhs) noexcept { return named_number_t<Number, Tag>{lhs.get() * rhs.get()}; }
    template <typename Number, typename Tag> constexpr named_number_t<Number, Tag> operator*(named_number_t<Number, Tag> lhs, Number rhs) noexcept { return named_number_t<Number, Tag>{lhs.get() * rhs}; }
    // no operator/

    template <typename Number, typename Tag> constexpr named_number_t<Number, Tag>& operator+=(named_number_t<Number, Tag>& lhs, named_number_t<Number, Tag> rhs) noexcept { lhs.get() = lhs.get() + rhs.get(); return lhs; }
    template <typename Number, typename Tag> constexpr named_number_t<Number, Tag>& operator+=(named_number_t<Number, Tag>& lhs, Number rhs) noexcept { lhs.get() = lhs.get() + rhs; return lhs; }
    template <typename Number, typename Tag> constexpr named_number_t<Number, Tag>& operator-=(named_number_t<Number, Tag>& lhs, named_number_t<Number, Tag> rhs) noexcept { lhs.get() = lhs.get() - rhs.get(); return lhs; }
    template <typename Number, typename Tag> constexpr named_number_t<Number, Tag>& operator-=(named_number_t<Number, Tag>& lhs, Number rhs) noexcept { lhs.get() = lhs.get() - rhs; return lhs; }
    template <typename Number, typename Tag> constexpr named_number_t<Number, Tag>& operator*=(named_number_t<Number, Tag>& lhs, named_number_t<Number, Tag> rhs) noexcept { lhs.get() = lhs.get() * rhs.get(); return lhs; }
    template <typename Number, typename Tag> constexpr named_number_t<Number, Tag>& operator*=(named_number_t<Number, Tag>& lhs, Number rhs) noexcept { lhs.get() = lhs.get() * rhs; return lhs; }
    // no operator/

    // ----------------------------------------------------------------------

    template <typename Tag> class named_size_t : public named_number_t<size_t, Tag>
    {
      public:
        using named_number_t<size_t, Tag>::named_number_t;
        constexpr named_size_t(named_number_t<size_t, Tag> src) : named_number_t<size_t, Tag>{src} {}
    };

    template <typename Tag> range(int, named_size_t<Tag>)->range<named_size_t<Tag>>;
    template <typename Tag> range(size_t, named_size_t<Tag>)->range<named_size_t<Tag>>;
    template <typename Tag> range(named_size_t<Tag>)->range<named_size_t<Tag>>;
    template <typename Tag> range(named_size_t<Tag>, named_size_t<Tag>)->range<named_size_t<Tag>>;

    // ----------------------------------------------------------------------

    template <typename Tag> class named_double_t : public named_number_t<double, Tag>
    {
      public:
        using named_number_t<double, Tag>::named_number_t;
        constexpr named_double_t(named_number_t<double, Tag> src) : named_number_t<double, Tag>{src} {}
    };

    template <typename Tag> constexpr bool operator==(const named_double_t<Tag>& lhs, const named_double_t<Tag>& rhs) noexcept { return float_equal(lhs.get(), rhs.get()); }

    // ----------------------------------------------------------------------

    template <typename Number, typename Tag> class named_number_from_string_t : public named_t<std::variant<Number, std::string_view>, Tag>
    {
      public:
        using base_t = named_t<std::variant<Number, std::string_view>, Tag>;

        template <typename TT> explicit constexpr named_number_from_string_t(TT&& value) : base_t(std::forward<TT>(value)) {}
        using base_t::get;

        constexpr Number as_number() const
        {
            return std::visit(
                []<typename Repr>(Repr content)->Number {
                    if constexpr (std::is_same_v<Repr, Number>) {
                        return content;
                    }
                    else {
                        if (content.empty())
                            return 0;
                        else
                            return ::string::from_chars<Number>(content);
                    }
                },
                get());
        }

        std::string as_string() const
        {
            return std::visit(
                []<typename Repr>(Repr content) -> std::string {
                    if constexpr (std::is_same_v<Repr, Number>)
                        return acmacs::to_string(content);
                    else
                        return std::string{content};
                },
                get());
        }

        constexpr bool is_zero() const
        {
            return std::visit(
                []<typename Repr>(Repr content)->bool {
                    if constexpr (std::is_same_v<Repr, double>) {
                        return float_zero(content);
                    }
                    else if constexpr (std::is_integral_v<Repr>) {
                        return content == 0;
                    }
                    else {
                        if (content.empty())
                            return true;
                        else
                            return float_zero(::string::from_chars<double>(content));
                    }
                },
                get());
        }

    };

    template <typename Number, typename Tag> constexpr bool operator<(const named_number_from_string_t<Number, Tag>& lhs, const named_number_from_string_t<Number, Tag>& rhs) noexcept { return lhs.as_number() < rhs.as_number(); }
    template <typename Number, typename Tag> constexpr bool operator<=(const named_number_from_string_t<Number, Tag>& lhs, const named_number_from_string_t<Number, Tag>& rhs) noexcept { return lhs.as_number() <= rhs.as_number(); }
    template <typename Number, typename Tag> constexpr bool operator>(const named_number_from_string_t<Number, Tag>& lhs, const named_number_from_string_t<Number, Tag>& rhs) noexcept { return lhs.as_number() > rhs.as_number(); }
    template <typename Number, typename Tag> constexpr bool operator>=(const named_number_from_string_t<Number, Tag>& lhs, const named_number_from_string_t<Number, Tag>& rhs) noexcept { return lhs.as_number() >= rhs.as_number(); }

    template <typename Number, typename Tag> constexpr named_number_from_string_t<Number, Tag> operator+(const named_number_from_string_t<Number, Tag>& lhs, const named_number_from_string_t<Number, Tag>& rhs) noexcept { return named_number_from_string_t<Number, Tag>{lhs.as_number() + rhs.as_number()}; }
    template <typename Number, typename Tag> constexpr named_number_from_string_t<Number, Tag> operator-(const named_number_from_string_t<Number, Tag>& lhs, const named_number_from_string_t<Number, Tag>& rhs) noexcept { return named_number_from_string_t<Number, Tag>{lhs.as_number() - rhs.as_number()}; }
    template <typename Number, typename Tag> constexpr named_number_from_string_t<Number, Tag> operator*(const named_number_from_string_t<Number, Tag>& lhs, const named_number_from_string_t<Number, Tag>& rhs) noexcept { return named_number_from_string_t<Number, Tag>{lhs.as_number() * rhs.as_number()}; }
    template <typename Number, typename Tag> constexpr named_number_from_string_t<Number, Tag> operator*(const named_number_from_string_t<Number, Tag>& lhs, double rhs) noexcept { return named_number_from_string_t<Number, Tag>{lhs.as_number() * rhs}; }
    // no operator/

    template <typename Number, typename Tag> constexpr named_number_from_string_t<Number, Tag>& operator+=(named_number_from_string_t<Number, Tag>& lhs, const named_number_from_string_t<Number, Tag>& rhs) noexcept { lhs.get() = lhs.as_number() + rhs.as_number(); return lhs; }
    template <typename Number, typename Tag> constexpr named_number_from_string_t<Number, Tag>& operator+=(named_number_from_string_t<Number, Tag>& lhs, Number rhs) noexcept { lhs.get() = lhs.as_number() + rhs; return lhs; }
    template <typename Number, typename Tag> constexpr named_number_from_string_t<Number, Tag>& operator-=(named_number_from_string_t<Number, Tag>& lhs, const named_number_from_string_t<Number, Tag>& rhs) noexcept { lhs.get() = lhs.as_number() - rhs.as_number(); return lhs; }
    template <typename Number, typename Tag> constexpr named_number_from_string_t<Number, Tag>& operator-=(named_number_from_string_t<Number, Tag>& lhs, Number rhs) noexcept { lhs.get() = lhs.as_number() - rhs; return lhs; }
    template <typename Number, typename Tag> constexpr named_number_from_string_t<Number, Tag>& operator*=(named_number_from_string_t<Number, Tag>& lhs, const named_number_from_string_t<Number, Tag>& rhs) noexcept { lhs.get() = lhs.as_number() * rhs.as_number(); return lhs; }
    template <typename Number, typename Tag> constexpr named_number_from_string_t<Number, Tag>& operator*=(named_number_from_string_t<Number, Tag>& lhs, Number rhs) noexcept { lhs.get() = lhs.as_number() * rhs; return lhs; }
    // no operator/

    template <typename Tag> using named_int_from_string_t  = named_number_from_string_t<int, Tag>;
    template <typename Tag> using named_size_t_from_string_t  = named_number_from_string_t<size_t, Tag>;
    template <typename Tag> using named_double_from_string_t  = named_number_from_string_t<double, Tag>;

    template <typename Tag> constexpr named_double_from_string_t<Tag> operator/(const named_double_from_string_t<Tag>& lhs, const named_double_from_string_t<Tag>& rhs) noexcept { return named_double_from_string_t<Tag>{lhs.as_number() / rhs.as_number()}; }
    template <typename Tag, typename Denom> constexpr named_double_from_string_t<Tag> operator/(const named_double_from_string_t<Tag>& lhs, Denom rhs) noexcept { return named_double_from_string_t<Tag>{lhs.as_number() / rhs}; }
    template <typename Tag> constexpr named_double_from_string_t<Tag>& operator*=(named_double_from_string_t<Tag>& lhs, const named_double_from_string_t<Tag>& rhs) noexcept { lhs.get() = lhs.as_number() / rhs.as_number(); return lhs; }

    // ----------------------------------------------------------------------

    template <typename T, typename Tag> class named_string_base_t : public named_t<T, Tag>
    {
      public:
        using named_t<T, Tag>::named_t;

        bool empty() const noexcept { return this->get().empty(); }
        size_t size() const noexcept { return this->get().size(); }
        char operator[](size_t pos) const { return this->get()[pos]; }
        size_t find(const char* s, size_t pos = 0) const noexcept { return this->get().find(s, pos); }
        size_t find(char ch, size_t pos = 0) const noexcept { return this->get().find(ch, pos); }
        constexpr operator std::string_view() const noexcept { return this->get(); }
        int compare(const named_string_base_t<T, Tag>& rhs) const { return ::string::compare(this->get(), rhs.get()); }
    };

    template <typename T, typename Tag> constexpr bool operator<(const named_string_base_t<T, Tag>& lhs, const named_string_base_t<T, Tag>& rhs) noexcept { return lhs.get() < rhs.get(); }
    template <typename T, typename Tag> constexpr bool operator<=(const named_string_base_t<T, Tag>& lhs, const named_string_base_t<T, Tag>& rhs) noexcept { return lhs.get() <= rhs.get(); }
    template <typename T, typename Tag> constexpr bool operator>(const named_string_base_t<T, Tag>& lhs, const named_string_base_t<T, Tag>& rhs) noexcept { return lhs.get() > rhs.get(); }
    template <typename T, typename Tag> constexpr bool operator>=(const named_string_base_t<T, Tag>& lhs, const named_string_base_t<T, Tag>& rhs) noexcept { return lhs.get() >= rhs.get(); }

    template <typename Tag> class named_string_t : public named_string_base_t<std::string, Tag>
    {
      public:
        using named_string_base_t<std::string, Tag>::named_string_base_t;

        void assign(std::string_view source) { this->get().assign(source); }
    };

    template <typename Tag> class named_string_view_t : public named_string_base_t<std::string_view, Tag>
    {
      public:
        using named_string_base_t<std::string_view, Tag>::named_string_base_t;
        template <typename Tag2> constexpr named_string_view_t(const named_string_t<Tag2>& src) : named_string_base_t<std::string_view, Tag>{*src} {}
    };

    // ----------------------------------------------------------------------

    template <typename T, typename Tag> class named_vector_t : public named_t<std::vector<T>, Tag>
    {
      public:
        using value_type = T;
        using named_t<std::vector<T>, Tag>::named_t;

        constexpr auto begin() const { return this->get().begin(); }
        constexpr auto end() const { return this->get().end(); }
        constexpr auto begin() { return this->get().begin(); }
        constexpr auto end() { return this->get().end(); }
        constexpr auto rbegin() { return this->get().rbegin(); }
        constexpr auto rend() { return this->get().rend(); }
        constexpr auto empty() const { return this->get().empty(); }
        constexpr auto size() const { return this->get().size(); }
        constexpr auto operator[](size_t index) const { return this->get()[index]; }
        constexpr auto& operator[](size_t index) { return this->get()[index]; }

        // for std::back_inserter
        constexpr void push_back(const T& val) { this->get().push_back(val); }
        constexpr void push_back(T&& val) { this->get().push_back(std::forward<T>(val)); }

        void remove(const T& val)
        {
            if (const auto found = std::find(begin(), end(), val); found != end())
                this->get().erase(found);
        }

        // set like
        bool exists(const T& val) const { return std::find(begin(), end(), val) != end(); }
        void insert_if_not_present(const T& val)
        {
            if (!exists(val))
                push_back(val);
        }
        void insert_if_not_present(T&& val)
        {
            if (!exists(std::forward<T>(val)))
                push_back(std::forward<T>(val));
        }
        void merge_in(const named_vector_t<T, Tag>& another)
        {
            for (const auto& val : another)
                insert_if_not_present(val);
        }
    };

    template <typename Tag> constexpr bool operator<(const named_vector_t<std::string, Tag>& lhs, const named_vector_t<std::string, Tag>& rhs) noexcept
    {
        const auto sz = std::min(lhs.size(), rhs.size());
        for (size_t i = 0; i < sz; ++i) {
            if (!(lhs[i] == rhs[i]))
                return lhs[i] < rhs[i];
        }
        if (lhs.size() < rhs.size())
            return true;
        return false;
    }

} // namespace acmacs

// ----------------------------------------------------------------------

template <typename Tag> struct fmt::formatter<acmacs::named_size_t<Tag>> : fmt::formatter<size_t> {
    template <typename FormatCtx> auto format(const acmacs::named_size_t<Tag>& nt, FormatCtx& ctx) { return fmt::formatter<size_t>::format(nt.get(), ctx); }
};

template <typename Tag> struct fmt::formatter<acmacs::named_double_t<Tag>> : fmt::formatter<double> {
    template <typename FormatCtx> auto format(const acmacs::named_double_t<Tag>& nt, FormatCtx& ctx) { return fmt::formatter<double>::format(nt.get(), ctx); }
};

template <typename T, typename Tag> struct fmt::formatter<acmacs::named_vector_t<T, Tag>> : fmt::formatter<std::vector<T>> {
    template <typename FormatCtx> auto format(const acmacs::named_vector_t<T, Tag>& vec, FormatCtx& ctx) { return fmt::formatter<std::vector<T>>::format(vec.get(), ctx); }
};

template <typename Number, typename Tag> struct fmt::formatter<acmacs::named_number_from_string_t<Number, Tag>> : public fmt::formatter<acmacs::fmt_default_formatter>
{
    template <typename FormatContext> auto format(const acmacs::named_number_from_string_t<Number, Tag>& num, FormatContext& ctx)
    {
        return std::visit(
            [&ctx]<typename Repr>(Repr content) { return format_to(ctx.out(), "{}", content); }, num.get());
    }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
