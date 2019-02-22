#pragma once

#include <limits>
#include <cassert>
#include <cmath>

#include "acmacs-base/vector.hh"
#include "acmacs-base/transformation.hh"
#include "acmacs-base/size.hh"
#include "acmacs-base/stream.hh"
#include "acmacs-base/indexes.hh"
#include "acmacs-base/range.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
//    class Coordinates : public Vector
//    {
//      public:
//        using Vector::Vector;
//
//        Coordinates(Location2D loc) : Vector{loc.x(), loc.y()} {}
//        Coordinates(Location3D loc) : Vector{loc.x(), loc.y(), loc.z()} {}
//
//        Coordinates transform(const Transformation& aTransformation) const
//        {
//            switch (size()) {
//                case 2:
//                    return aTransformation.transform(static_cast<Location2D>(*this));
//                case 3:
//                    return aTransformation.transform(static_cast<Location3D>(*this));
//            }
//            return {};
//        }
//
//        bool not_nan() const
//        {
//            return !empty() && std::all_of(begin(), end(), [](double value) -> bool { return !std::isnan(value); });
//        }
//
//        operator Location2D() const
//        {
//            assert(size() == 2);
//            return {operator[](0), operator[](1)};
//        }
//
//        operator Location3D() const
//        {
//            assert(size() == 3);
//            return {operator[](0), operator[](1), operator[](2)};
//        }
//
//        Coordinates& mean_with(const Coordinates& another)
//        {
//            for (size_t dim = 0; dim < size(); ++dim)
//                (*this)[dim] = ((*this)[dim] + another[dim]) / 2.0;
//            return *this;
//        }
//
//    }; // class Coordinates
//
//    inline std::ostream& operator<<(std::ostream& s, const Coordinates& c)
//    {
//        stream_internal::write_to_stream(s, c, "[", "]", ", ");
//        return s;
//    }

    // ----------------------------------------------------------------------

    struct Area
    {
        PointCoordinates min, max;

        Area(const PointCoordinates& a_min, const PointCoordinates& a_max) : min(a_min), max(a_max) {}
        Area(const PointCoordinates& a_min) : min(a_min), max(a_min) {}

        size_t num_dim() const { return min.number_of_dimensions(); }

        void extend(const PointCoordinates& point)
        {
            for (size_t dim = 0; dim < num_dim(); ++dim) {
                min[dim] = std::min(point[dim], min[dim]);
                max[dim] = std::max(point[dim], max[dim]);
            }
        }

        double area() const
        {
            double result = max[0] - min[0];
            for (size_t dim = 1; dim < num_dim(); ++dim)
                result *= max[dim] - min[dim];
            return result;
        }

        class Iterator
        {
         private:
            double step_;
            PointCoordinates min_, max_, current_;

            friend struct Area;
            Iterator(double step, const PointCoordinates& a_min, const PointCoordinates& a_max) : step_(step), min_(a_min), max_(a_min), current_(a_min) { set_max(a_max); }
            Iterator() : step_(std::numeric_limits<double>::quiet_NaN()) {}

            void set_max(const PointCoordinates& a_max)
                {
                    for (auto dim : acmacs::range(max_.number_of_dimensions()))
                        max_[dim] = min_[dim] + std::ceil((a_max[dim] - min_[dim]) / step_) * step_;
                }

          public:
            bool operator==(const Iterator& rhs) const
                {
                    if (std::isnan(rhs.step_))
                        return current_[0] > max_[0];
                    else if (std::isnan(step_))
                        return rhs.current_[0] > rhs.max_[0];
                    else
                        throw std::runtime_error("cannot compare Area::Iterators");
                }

            bool operator!=(const Iterator& rhs) const { return !operator==(rhs); }
            const PointCoordinates& operator*() const { return current_; }
            const PointCoordinates* operator->() const { return &current_; }

            const Iterator& operator++()
                {
                    if (current_[0] <= max_[0]) {
                        for (auto dim : acmacs::range(current_.number_of_dimensions())) {
                            current_[dim] += step_;
                            if (current_[dim] <= max_[dim]) {
                                std::copy(min_.begin(), min_.begin() + (dim), current_.begin());
                                break;
                            }
                        }
                    }
                    return *this;
                }
        };

        Iterator begin(double step) const { return Iterator(step, min, max); }
        Iterator end() const { return Iterator{}; }

    }; // struct Area

    inline std::string to_string(const Area& area, size_t precision = 32) { return acmacs::to_string(area.min, precision) + ' ' + acmacs::to_string(area.max, precision); }

    inline std::ostream& operator<<(std::ostream& s, const Area& area) { return s << to_string(area, 4); }

      // ----------------------------------------------------------------------

    class Layout;

    class LayoutConstIterator
    {
      public:
        PointCoordinates operator*() const;
        auto& operator++()
        {
            ++point_no_;
            // do not skip disconnected points to avoid jumping over end iterator
            return *this;
        }
        bool operator==(const LayoutConstIterator& rhs) const
        {
            if (&parent_ != &rhs.parent_)
                throw std::runtime_error("LayoutDimensionConstIterator: cannot compare iterators for different layouts");
            return point_no_ == rhs.point_no_;
        }
        bool operator!=(const LayoutConstIterator& rhs) const { return !operator==(rhs); }

      private:
        const Layout& parent_;
        mutable size_t point_no_;

        LayoutConstIterator(const Layout& parent, size_t point_no) : parent_{parent}, point_no_{point_no} {}

        friend class Layout;

    }; // class LayoutConstIterator

    class LayoutDimensionConstIterator
    {
      public:
        double operator*() const;
        auto& operator++()
        {
            ++point_no_;
            // do not skip disconnected points to avoid jumping over end iterator
            return *this;
        }
        bool operator==(const LayoutDimensionConstIterator& rhs) const
        {
            if (&parent_ != &rhs.parent_)
                throw std::runtime_error("LayoutDimensionConstIterator: cannot compare iterators for different layouts");
            if (dimension_no_ != rhs.dimension_no_)
                throw std::runtime_error("LayoutDimensionConstIterator: cannot compare iterators for dimensions");
            return point_no_ == rhs.point_no_;
        }
        bool operator!=(const LayoutDimensionConstIterator& rhs) const { return !operator==(rhs); }

      private:
        const Layout& parent_;
        mutable size_t point_no_;
        const size_t dimension_no_;

        LayoutDimensionConstIterator(const Layout& parent, size_t point_no, size_t dimension_no) : parent_{parent}, point_no_{point_no}, dimension_no_{dimension_no} {}

        friend class Layout;

    }; // class LayoutDimensionConstIterator

    // ----------------------------------------------------------------------

    class Layout : public std::vector<double>
    {
      public:
        using Vec = std::vector<double>;

        Layout() = default;
        Layout(const Layout&) = default;
        Layout(Layout&&) = default;
        Layout(size_t number_of_points, size_t number_of_dimensions) : Vec(number_of_points * number_of_dimensions, std::numeric_limits<double>::quiet_NaN()), number_of_dimensions_{number_of_dimensions} {}
        Layout(size_t number_of_dimensions, const double* first, const double* last) : Vec(first, last), number_of_dimensions_{number_of_dimensions} {}
        Layout(const Layout& source, const std::vector<size_t>& indexes);
        virtual ~Layout() = default;
        Layout& operator=(const Layout&) = default;
        Layout& operator=(Layout&&) = default;

        constexpr size_t number_of_points() const noexcept { return size() / number_of_dimensions_; }
        constexpr size_t number_of_dimensions() const noexcept { return number_of_dimensions_; }

        void change_number_of_dimensions(size_t num_dim, bool allow_dimensions_increase = false)
        {
            if (!allow_dimensions_increase && num_dim >= number_of_dimensions_)
                throw std::runtime_error("Layout::change_number_of_dimensions: dimensions increase: " + std::to_string(number_of_dimensions_) + " --> " + std::to_string(num_dim));
            resize(number_of_points() * num_dim);
            number_of_dimensions_ = num_dim;
        }

        const PointCoordinates operator[](size_t point_no) const
        {
            return {Vec::begin() + static_cast<difference_type>(point_no * number_of_dimensions_), Vec::begin() + static_cast<difference_type>((point_no + 1) * number_of_dimensions_)};
        }

        PointCoordinates operator[](size_t point_no)
        {
            return {Vec::begin() + static_cast<difference_type>(point_no * number_of_dimensions_), Vec::begin() + static_cast<difference_type>((point_no + 1) * number_of_dimensions_)};
        }

        PointCoordinates get(size_t point_no) const { return operator[](point_no); }

        double operator()(size_t point_no, size_t aDimensionNo) const { return Vec::operator[](point_no * number_of_dimensions_ + aDimensionNo); }
        double& operator()(size_t point_no, size_t aDimensionNo) { return Vec::operator[](point_no * number_of_dimensions_ + aDimensionNo); }
        double coordinate(size_t point_no, size_t aDimensionNo) const { return operator()(point_no, aDimensionNo); }
        double& coordinate(size_t point_no, size_t aDimensionNo) { return operator()(point_no, aDimensionNo); }
        bool point_has_coordinates(size_t point_no) const { return operator[](point_no).not_nan(); }
        const std::vector<double>& as_flat_vector_double() const { return *this; }

        void set_nan(size_t point_no)
        {
            const auto first{Vec::begin() + static_cast<difference_type>(point_no * number_of_dimensions())}, last{first + static_cast<difference_type>(number_of_dimensions())};
            std::for_each(first, last, [](auto& target) { target = std::numeric_limits<std::decay_t<decltype(target)>>::quiet_NaN(); });
        }

        void remove_points(const ReverseSortedIndexes& indexes, size_t base)
        {
            for (const auto index : indexes) {
                const auto first = Vec::begin() + static_cast<difference_type>((index + base) * number_of_dimensions_);
                erase(first, first + static_cast<difference_type>(number_of_dimensions_));
            }
        }

        void insert_point(size_t before, size_t base)
        {
            insert(Vec::begin() + static_cast<difference_type>((before + base) * number_of_dimensions_), number_of_dimensions_, std::numeric_limits<double>::quiet_NaN());
        }

        std::vector<std::pair<double, double>> minmax() const;

        double distance(size_t p1, size_t p2, double no_distance = std::numeric_limits<double>::quiet_NaN()) const
        {
            if (const auto c1 = operator[](p1), c2 = operator[](p2); c1.not_nan() && c2.not_nan())
                return acmacs::distance(c1, c2);
            else
                return no_distance;
        }

        // returns indexes for min points for each dimension and max points for each dimension
        std::pair<std::vector<size_t>, std::vector<size_t>> min_max_point_indexes() const;
        // returns boundary coordinates (min and max)
        Area area() const;                                  // for all points
        Area area(const std::vector<size_t>& points) const; // just for the specified point indexes
        std::shared_ptr<Layout> transform(const Transformation& aTransformation) const;
        PointCoordinates centroid() const;

        LayoutConstIterator begin() const { return {*this, 0}; }
        LayoutConstIterator end() const { return {*this, number_of_points()}; }
        LayoutConstIterator begin_antigens(size_t /*number_of_antigens*/) const { return {*this, 0}; }
        LayoutConstIterator end_antigens(size_t number_of_antigens) const { return {*this, number_of_antigens}; }
        LayoutConstIterator begin_sera(size_t number_of_antigens) const { return {*this, number_of_antigens}; }
        LayoutConstIterator end_sera(size_t /*number_of_antigens*/) const { return {*this, number_of_points()}; }

        LayoutDimensionConstIterator begin_dimension(size_t dimension_no) const { return {*this, 0, dimension_no}; }
        LayoutDimensionConstIterator end_dimension(size_t dimension_no) const { return {*this, number_of_points(), dimension_no}; }
        LayoutDimensionConstIterator begin_antigens_dimension(size_t /*number_of_antigens*/, size_t dimension_no) const { return {*this, 0, dimension_no}; }
        LayoutDimensionConstIterator end_antigens_dimension(size_t number_of_antigens, size_t dimension_no) const { return {*this, number_of_antigens, dimension_no}; }
        LayoutDimensionConstIterator begin_sera_dimension(size_t number_of_antigens, size_t dimension_no) const { return {*this, number_of_antigens, dimension_no}; }
        LayoutDimensionConstIterator end_sera_dimension(size_t /*number_of_antigens*/, size_t dimension_no) const { return {*this, number_of_points(), dimension_no}; }

      private:
        size_t number_of_dimensions_;

    }; // class Layout

    inline std::ostream& operator<<(std::ostream& s, const Layout& aLayout)
    {
        s << "Layout [";
        for (size_t no = 0; no < aLayout.number_of_points(); ++no)
            s << aLayout[no] << ' ';
        s << ']';
        return s;
    }

    inline PointCoordinates LayoutConstIterator::operator*() const
    {
        while (point_no_ < parent_.number_of_points() && !parent_.point_has_coordinates(point_no_))
            ++point_no_; // skip disconnected points
        return parent_.get(point_no_);
    }

    inline double LayoutDimensionConstIterator::operator*() const
    {
        while (point_no_ < parent_.number_of_points() && !parent_.point_has_coordinates(point_no_))
            ++point_no_; // skip disconnected points
        return parent_.coordinate(point_no_, dimension_no_);
    }


} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
