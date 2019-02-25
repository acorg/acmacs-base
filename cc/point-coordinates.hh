#pragma once

#include <iostream>
// #include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <limits>
#include <array>
#include <variant>

#include "acmacs-base/float.hh"
#include "acmacs-base/to-string.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    class PointCoordinates
    {
      public:
        enum zero2D { zero2D };
        enum zero3D { zero3D };

        explicit PointCoordinates(size_t number_of_dimensions)
        {
            const auto nan = std::numeric_limits<double>::quiet_NaN();
            switch (number_of_dimensions) {
              case 2:
                  data_ = Store2D{nan, nan};
                  break;
              case 3:
                  data_ = Store3D{nan, nan, nan};
                  break;
              default:
                  assert(number_of_dimensions == 2);
                  break;
            }
        }
        PointCoordinates(double x, double y) : data_(Store2D{x, y}) {}
        PointCoordinates(double x, double y, double z) : data_(Store3D{x, y, z}) {}
        PointCoordinates(enum zero2D) : PointCoordinates(0.0, 0.0) {}
        PointCoordinates(enum zero3D) : PointCoordinates(0.0, 0.0, 0.0) {}
        PointCoordinates(const double* first, const double* last) : data_(ConstRef{first, static_cast<size_t>(last - first)}) {}
        PointCoordinates(double* first, double* last) : data_(Ref{first, static_cast<size_t>(last - first)}) {}
        PointCoordinates(std::vector<double>::const_iterator first, std::vector<double>::const_iterator last) : PointCoordinates(&*first, &*last) {}
        PointCoordinates(std::vector<double>::iterator first, std::vector<double>::iterator last) : PointCoordinates(&*first, &*last) {}

        PointCoordinates(const PointCoordinates& rhs)
        {
            switch (rhs.number_of_dimensions()) {
              case 2:
                  data_ = Store2D{rhs[0], rhs[1]};
                  break;
              case 3:
                  data_ = Store3D{rhs[0], rhs[1], rhs[2]};
                  break;
              default:
                  assert(number_of_dimensions() == 2);
                  break;
            }
        }

        PointCoordinates(PointCoordinates&& rhs) : PointCoordinates(rhs) {}

        PointCoordinates& operator=(const PointCoordinates& rhs)
        {
            assert(number_of_dimensions() == rhs.number_of_dimensions());
            std::copy(rhs.begin(), rhs.end(), begin());
            return *this;
        }

        PointCoordinates& operator=(PointCoordinates&& rhs) { return operator=(rhs); }

        bool operator==(const PointCoordinates& rhs) const { return std::equal(begin(), end(), rhs.begin(), rhs.end()); }
        bool operator!=(const PointCoordinates& rhs) const { return !operator==(rhs); }

        constexpr size_t number_of_dimensions() const
        {
            return std::visit(
                [](auto&& data) -> size_t {
                    using T = std::decay_t<decltype(data)>;
                    if constexpr (std::is_same_v<T, Store2D>)
                        return 2;
                    else if constexpr (std::is_same_v<T, Store3D>)
                        return 3;
                    else
                        return data.size;
                },
                data_);
        }

        constexpr double operator[](size_t dim) const
        { /* assert(dim < number_of_dimensions()); */
            return std::visit(
                [dim](auto&& data) -> double {
                    using T = std::decay_t<decltype(data)>;
                    if constexpr (std::is_same_v<T, Store2D> || std::is_same_v<T, Store3D>)
                        return data[dim];
                    else
                        return data.begin[dim];
                },
                data_);
        }

        constexpr double& operator[](size_t dim)
        { /* assert(dim < number_of_dimensions()); */
            return std::visit(
                [dim](auto&& data) -> double& {
                    using T = std::decay_t<decltype(data)>;
                    if constexpr (std::is_same_v<T, Store2D> || std::is_same_v<T, Store3D>)
                        return data[dim];
                    else if constexpr (std::is_same_v<T, ConstRef>)
                        throw std::runtime_error("Cannot update const PointCoordinates");
                    else
                        return data.begin[dim];
                },
                data_);
        }

        constexpr const double* begin() const
        {
            return std::visit(
                [](auto&& data) -> const double* {
                    using T = std::decay_t<decltype(data)>;
                    if constexpr (std::is_same_v<T, Store2D> || std::is_same_v<T, Store3D>)
                        return data.cbegin();
                    else
                        return data.begin;
                },
                data_);
        }
        constexpr const double* end() const
        {
            return std::visit(
                [](auto&& data) -> const double* {
                    using T = std::decay_t<decltype(data)>;
                    if constexpr (std::is_same_v<T, Store2D> || std::is_same_v<T, Store3D>)
                        return data.cend();
                    else
                        return data.begin + data.size;
                },
                data_);
        }

        constexpr double* begin()
        {
            return std::visit(
                [](auto&& data) -> double* {
                    using T = std::decay_t<decltype(data)>;
                    if constexpr (std::is_same_v<T, Store2D> || std::is_same_v<T, Store3D>)
                        return data.begin();
                    else if constexpr (std::is_same_v<T, ConstRef>)
                        throw std::runtime_error("Cannot update const PointCoordinates");
                    else
                        return data.begin;
                },
                data_);
        }
        constexpr double* end()
        {
            return std::visit(
                [](auto&& data) -> double* {
                    using T = std::decay_t<decltype(data)>;
                    if constexpr (std::is_same_v<T, Store2D> || std::is_same_v<T, Store3D>)
                        return data.end();
                    else if constexpr (std::is_same_v<T, ConstRef>)
                        throw std::runtime_error("Cannot update const PointCoordinates");
                    else
                        return data.begin + data.size;
                },
                data_);
        }

        constexpr double x() const { return operator[](0); }
        constexpr double y() const { return operator[](1); }
        constexpr double z() const { return operator[](2); }

        constexpr void x(double val) { operator[](0) = val; }
        constexpr void y(double val) { operator[](1) = val; }
        constexpr void z(double val) { operator[](2) = val; }

        PointCoordinates& operator+=(const PointCoordinates& rhs) { std::transform(begin(), end(), rhs.begin(), begin(), [](double v1, double v2) { return v1 + v2; }); return *this; }
        PointCoordinates& operator+=(double val) { std::transform(begin(), end(), begin(), [val](double v1) { return v1 + val; }); return *this; }
        PointCoordinates& operator-=(const PointCoordinates& rhs) { std::transform(begin(), end(), rhs.begin(), begin(), [](double v1, double v2) { return v1 - v2; }); return *this; }
        PointCoordinates& operator*=(double val) { std::transform(begin(), end(), begin(), [val](double v1) { return v1 * val; }); return *this; }
        PointCoordinates& operator/=(double val) { std::transform(begin(), end(), begin(), [val](double v1) { return v1 / val; }); return *this; }

        bool empty() const { return std::any_of(begin(), end(), [](auto val) { return std::isnan(val); }); }
        bool exists() const { return !empty(); }

      private:
        using Store2D = std::array<double, 2>;
        using Store3D = std::array<double, 3>;
        struct Ref { double* begin; size_t size; };
        struct ConstRef { const double* const begin; size_t size; };

        std::variant<Store2D, Store3D, Ref, ConstRef> data_;

    }; // class PointCoordinates

    inline std::string to_string(const PointCoordinates& coord, size_t precision = 32)
    {
        auto result = '{' + acmacs::to_string(coord.x(), precision) + ", " + acmacs::to_string(coord.y(), precision);
        if (coord.number_of_dimensions() == 3)
            result += ", " + to_string(coord.z(), precision);
        return result + '}';
    }

    inline std::ostream& operator<<(std::ostream& out, const PointCoordinates& coord) { return out << to_string(coord); }

    inline double distance2(const PointCoordinates& p1, const PointCoordinates& p2)
    {
        std::vector<double> diff(p1.number_of_dimensions());
        std::transform(p1.begin(), p1.end(), p2.begin(), diff.begin(), [](double v1, double v2) { return v1 - v2; });
        return std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    }

    inline double distance(const PointCoordinates& p1, const PointCoordinates& p2) { return std::sqrt(distance2(p1, p2)); }
    inline PointCoordinates middle(const PointCoordinates& p1, const PointCoordinates& p2)
    {
           PointCoordinates result(p1.number_of_dimensions());
           std::transform(p1.begin(), p1.end(), p2.begin(), result.begin(), [](double v1, double v2) { return (v1 + v2) / 2.0; });
           return result;
    }

    inline PointCoordinates operator-(const PointCoordinates& p1) { PointCoordinates result(p1); std::transform(result.begin(), result.end(), result.begin(), [](double val) { return -val; }); return result; }
    inline PointCoordinates operator+(const PointCoordinates& p1, const PointCoordinates& p2) { PointCoordinates result(p1); result += p2; return result; }
    inline PointCoordinates operator+(const PointCoordinates& p1, double val) { PointCoordinates result(p1); result += val; return result; }
    inline PointCoordinates operator-(const PointCoordinates& p1, const PointCoordinates& p2) { PointCoordinates result(p1); result -= p2; return result; }
    inline PointCoordinates operator-(const PointCoordinates& p1, double val) { PointCoordinates result(p1); result += -val; return result; }
    inline PointCoordinates operator*(const PointCoordinates& p1, double val) { PointCoordinates result(p1); result *= val; return result; }
    inline PointCoordinates operator/(const PointCoordinates& p1, double val) { PointCoordinates result(p1); result /= val; return result; }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
