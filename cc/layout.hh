#pragma once

#include <limits>
#include <cassert>
#include <cmath>
#include <memory>

#include "acmacs-base/number-of-dimensions.hh"
#include "acmacs-base/transformation.hh"
#include "acmacs-base/size.hh"
#include "acmacs-base/stream.hh"
#include "acmacs-base/indexes.hh"
#include "acmacs-base/range.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    struct Area
    {
        PointCoordinates min, max;

        Area(const PointCoordinates& a_min, const PointCoordinates& a_max) : min(a_min), max(a_max) {}
        Area(const PointCoordinates& a_min) : min(a_min), max(a_min) {}

        number_of_dimensions_t num_dim() const { return min.number_of_dimensions(); }

        void extend(const PointCoordinates& point)
        {
            for (number_of_dimensions_t dim{0}; dim < num_dim(); ++dim) {
                min[dim] = std::min(point[dim], min[dim]);
                max[dim] = std::max(point[dim], max[dim]);
            }
        }

        double area() const
        {
            double result = max.x() - min.x();
            for (number_of_dimensions_t dim{1}; dim < num_dim(); ++dim)
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
            Iterator() : step_(std::numeric_limits<double>::quiet_NaN()), min_{number_of_dimensions_t{2}}, max_{number_of_dimensions_t{2}}, current_{number_of_dimensions_t{2}} {}

            void set_max(const PointCoordinates& a_max)
                {
                    for (number_of_dimensions_t dim{0}; dim < a_max.number_of_dimensions(); ++dim)
                        max_[dim] = min_[dim] + std::ceil((a_max[dim] - min_[dim]) / step_) * step_;
                }

          public:
            bool operator==(const Iterator& rhs) const
                {
                    if (std::isnan(rhs.step_))
                        return current_.x() > max_.x();
                    else if (std::isnan(step_))
                        return rhs.current_.x() > rhs.max_.x();
                    else
                        throw std::runtime_error("cannot compare Area::Iterators");
                }

            bool operator!=(const Iterator& rhs) const { return !operator==(rhs); }
            const PointCoordinates& operator*() const { return current_; }
            const PointCoordinates* operator->() const { return &current_; }

            const Iterator& operator++()
                {
                    if (current_.x() <= max_.x()) {
                        for (number_of_dimensions_t dim{0}; dim < current_.number_of_dimensions(); ++dim) {
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
        const number_of_dimensions_t dimension_no_;

        LayoutDimensionConstIterator(const Layout& parent, size_t point_no, number_of_dimensions_t dimension_no) : parent_{parent}, point_no_{point_no}, dimension_no_{dimension_no} {}

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
        Layout(size_t number_of_points, number_of_dimensions_t number_of_dimensions) : Vec(number_of_points * number_of_dimensions, std::numeric_limits<double>::quiet_NaN()), number_of_dimensions_{number_of_dimensions} {}
        Layout(number_of_dimensions_t number_of_dimensions, const double* first, const double* last) : Vec(first, last), number_of_dimensions_{number_of_dimensions} {}
        Layout(const Layout& source, const std::vector<size_t>& indexes);
        virtual ~Layout() = default;
        Layout& operator=(const Layout&) = default;
        Layout& operator=(Layout&&) = default;

        size_t number_of_points() const noexcept { return size() / number_of_dimensions_; }
        constexpr number_of_dimensions_t number_of_dimensions() const noexcept { return number_of_dimensions_; }

        void change_number_of_dimensions(number_of_dimensions_t num_dim, bool allow_dimensions_increase = false)
        {
            if (!allow_dimensions_increase && num_dim >= number_of_dimensions_)
                throw std::runtime_error("Layout::change_number_of_dimensions: dimensions increase: " + acmacs::to_string(number_of_dimensions_) + " --> " + acmacs::to_string(num_dim));
            resize(number_of_points() * num_dim);
            number_of_dimensions_ = num_dim;
        }

        const PointCoordinates operator[](size_t point_no) const
        {
            return PointCoordinates(Vec::begin() + static_cast<difference_type>(point_no * number_of_dimensions_), Vec::begin() + static_cast<difference_type>((point_no + 1) * number_of_dimensions_));
        }

        PointCoordinates operator[](size_t point_no)
        {
            return PointCoordinates(Vec::begin() + static_cast<difference_type>(point_no * number_of_dimensions_), Vec::begin() + static_cast<difference_type>((point_no + 1) * number_of_dimensions_));
        }

        PointCoordinates get(size_t point_no) const { return operator[](point_no); }

        double operator()(size_t point_no, number_of_dimensions_t aDimensionNo) const { return Vec::operator[](point_no * number_of_dimensions_ + aDimensionNo); }
        double& operator()(size_t point_no, number_of_dimensions_t aDimensionNo) { return Vec::operator[](point_no * number_of_dimensions_ + aDimensionNo); }
        double coordinate(size_t point_no, number_of_dimensions_t aDimensionNo) const { return operator()(point_no, aDimensionNo); }
        double& coordinate(size_t point_no, number_of_dimensions_t aDimensionNo) { return operator()(point_no, aDimensionNo); }
        bool point_has_coordinates(size_t point_no) const { return operator[](point_no).exists(); }
        const std::vector<double>& as_flat_vector_double() const { return *this; }

        void set_nan(size_t point_no)
        {
            const auto first{Vec::begin() + static_cast<difference_type>(point_no * number_of_dimensions())}, last{first + static_cast<difference_type>(*number_of_dimensions())};
            std::for_each(first, last, [](auto& target) { target = std::numeric_limits<std::decay_t<decltype(target)>>::quiet_NaN(); });
        }

        void remove_points(const ReverseSortedIndexes& indexes, size_t base)
        {
            for (const auto index : indexes) {
                const auto first = Vec::begin() + static_cast<difference_type>((index + base) * number_of_dimensions_);
                erase(first, first + static_cast<difference_type>(*number_of_dimensions_));
            }
        }

        void insert_point(size_t before, size_t base)
        {
            insert(Vec::begin() + static_cast<difference_type>((before + base) * number_of_dimensions_), *number_of_dimensions_, std::numeric_limits<double>::quiet_NaN());
        }

        std::vector<std::pair<double, double>> minmax() const;

        double distance(size_t p1, size_t p2, double no_distance = std::numeric_limits<double>::quiet_NaN()) const
        {
            if (const auto c1 = operator[](p1), c2 = operator[](p2); c1.exists() && c2.exists())
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

        LayoutDimensionConstIterator begin_dimension(number_of_dimensions_t dimension_no) const { return {*this, 0, dimension_no}; }
        LayoutDimensionConstIterator end_dimension(number_of_dimensions_t dimension_no) const { return {*this, number_of_points(), dimension_no}; }
        LayoutDimensionConstIterator begin_antigens_dimension(size_t /*number_of_antigens*/, number_of_dimensions_t dimension_no) const { return {*this, 0, dimension_no}; }
        LayoutDimensionConstIterator end_antigens_dimension(size_t number_of_antigens, number_of_dimensions_t dimension_no) const { return {*this, number_of_antigens, dimension_no}; }
        LayoutDimensionConstIterator begin_sera_dimension(size_t number_of_antigens, number_of_dimensions_t dimension_no) const { return {*this, number_of_antigens, dimension_no}; }
        LayoutDimensionConstIterator end_sera_dimension(size_t /*number_of_antigens*/, number_of_dimensions_t dimension_no) const { return {*this, number_of_points(), dimension_no}; }

      private:
        number_of_dimensions_t number_of_dimensions_{2};

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
