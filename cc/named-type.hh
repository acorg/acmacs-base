#pragma once

#include "acmacs-base/string.hh"
#include "acmacs-base/fmt.hh"
#include "acmacs-base/to-string.hh"
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
        template <typename T2> explicit constexpr named_t(T2&& value) : value_(std::forward<T2>(value)) {}

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

    template <typename Tag> class named_size_t : public named_t<size_t, Tag>
    {
      public:
        template <typename T> explicit constexpr named_size_t(T&& value) : named_t<size_t, Tag>(static_cast<size_t>(std::forward<T>(value))) {}

        constexpr auto& operator++() { ++this->get(); return *this; }
        constexpr auto& operator--() { --this->get(); return *this; }
    };

    template <typename T> constexpr bool operator<(const named_size_t<T>& lhs, const named_size_t<T>& rhs) noexcept { return lhs.get() < rhs.get(); }
    template <typename T> constexpr bool operator<=(const named_size_t<T>& lhs, const named_size_t<T>& rhs) noexcept { return lhs.get() <= rhs.get(); }
    template <typename T> constexpr bool operator>(const named_size_t<T>& lhs, const named_size_t<T>& rhs) noexcept { return lhs.get() > rhs.get(); }
    template <typename T> constexpr bool operator>=(const named_size_t<T>& lhs, const named_size_t<T>& rhs) noexcept { return lhs.get() >= rhs.get(); }

    template <typename Tag> range(int, named_size_t<Tag>) -> range<named_size_t<Tag>>;
    template <typename Tag> range(size_t, named_size_t<Tag>) -> range<named_size_t<Tag>>;
    template <typename Tag> range(named_size_t<Tag>) -> range<named_size_t<Tag>>;
    template <typename Tag> range(named_size_t<Tag>, named_size_t<Tag>) -> range<named_size_t<Tag>>;

    // ----------------------------------------------------------------------

    template <typename Tag> class named_string_t : public named_t<std::string, Tag>
    {
      public:
        using named_t<std::string, Tag>::named_t;

        bool empty() const noexcept { return this->get().empty(); }
        size_t size() const noexcept { return this->get().size(); }
        char operator[](size_t pos) const { return this->get()[pos]; }
        size_t find(const char* s, size_t pos = 0) const noexcept { return this->get().find(s, pos); }
        size_t find(char ch, size_t pos = 0) const noexcept { return this->get().find(ch, pos); }
        constexpr operator std::string_view() const noexcept { return this->get(); }
        int compare(const named_string_t<Tag>& rhs) const { return ::string::compare(this->get(), rhs.get()); }
        void assign(std::string_view source) { this->get().assign(source); }
    };

    template <typename T> constexpr bool operator==(const named_string_t<T>& lhs, const named_string_t<T>& rhs) noexcept { return lhs.get() == rhs.get(); }
    template <typename T> constexpr bool operator==(const named_string_t<T>& lhs, const std::string& rhs) noexcept { return lhs.get() == rhs; }
    template <typename T> constexpr bool operator==(const named_string_t<T>& lhs, const char* rhs) noexcept { return lhs.get() == rhs; }
    template <typename T> constexpr bool operator==(const named_string_t<T>& lhs, std::string_view rhs) noexcept { return lhs.get() == rhs; }
    template <typename T, typename R> constexpr bool operator!=(const named_string_t<T>& lhs, R&& rhs) noexcept { return !operator==(lhs, std::forward<R>(rhs)); }
    template <typename T> constexpr bool operator<(const named_string_t<T>& lhs, const named_string_t<T>& rhs) noexcept { return lhs.get() < rhs.get(); }

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

        // for std::back_inserter
        constexpr void push_back(const T& val) { this->get().push_back(val); }
        constexpr void push_back(T&& val) { this->get().push_back(std::forward<T>(val)); }

        void remove(const T& val) { if (const auto found = std::find(begin(), end(), val); found != end()) this->get().erase(found); }

        // set like
        bool exists(const T& val) const { return std::find(begin(), end(), val) != end(); }
        void insert_if_not_present(const T& val) { if (!exists(val)) push_back(val); }
        void insert_if_not_present(T&& val) { if (!exist(val)) push_back(std::forward<T>(val)); }
        void merge_in(const named_vector_t<T, Tag>& another)
        {
            for (const auto& val : another)
                insert_if_not_present(val);
        }
    };

} // namespace acmacs

// ----------------------------------------------------------------------

// template <typename T, typename Tag> struct fmt::formatter<acmacs::named_t<T, Tag>> : fmt::formatter<T> {
//     template <typename FormatCtx> auto format(const acmacs::named_t<T, Tag>& nt, FormatCtx& ctx) { return fmt::formatter<T>::format(nt.get(), ctx); }
// };

template <typename Tag> struct fmt::formatter<acmacs::named_size_t<Tag>> : fmt::formatter<size_t> {
    template <typename FormatCtx> auto format(const acmacs::named_size_t<Tag>& nt, FormatCtx& ctx) { return fmt::formatter<size_t>::format(nt.get(), ctx); }
};

template <typename Tag> struct fmt::formatter<acmacs::named_string_t<Tag>> : fmt::formatter<std::string> {
    template <typename FormatCtx> auto format(const acmacs::named_string_t<Tag>& nt, FormatCtx& ctx) { return fmt::formatter<std::string>::format(nt.get(), ctx); }
};

template <typename T, typename Tag> struct fmt::formatter<acmacs::named_vector_t<T, Tag>> : fmt::formatter<std::vector<T>> {
    template <typename FormatCtx> auto format(const acmacs::named_vector_t<T, Tag>& vec, FormatCtx& ctx) { return fmt::formatter<std::vector<T>>::format(vec.get(), ctx); }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
