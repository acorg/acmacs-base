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
    class Coordinates : public Vector
    {
      public:
        using Vector::Vector;

        Coordinates(Location2D loc) : Vector{loc.x(), loc.y()} {}
        Coordinates(Location3D loc) : Vector{loc.x(), loc.y(), loc.z()} {}

        Coordinates transform(const Transformation& aTransformation) const
        {
            return aTransformation.transform(static_cast<Location2D>(*this));
        }

        bool not_nan() const
        {
            return !empty() && std::all_of(begin(), end(), [](double value) -> bool { return !std::isnan(value); });
        }

        operator Location2D() const
        {
            assert(size() == 2);
            return {operator[](0), operator[](1)};
        }

        operator Location3D() const
        {
            assert(size() == 3);
            return {operator[](0), operator[](1), operator[](2)};
        }

        Coordinates& mean_with(const Coordinates& another)
        {
            for (size_t dim = 0; dim < size(); ++dim)
                (*this)[dim] = ((*this)[dim] + another[dim]) / 2.0;
            return *this;
        }

    }; // class Coordinates

    inline std::ostream& operator<<(std::ostream& s, const Coordinates& c)
    {
        stream_internal::write_to_stream(s, c, "[", "]", ", ");
        return s;
    }

    // ----------------------------------------------------------------------

    struct Area
    {
        Coordinates min, max;

        Area(const Coordinates& a_min, const Coordinates& a_max) : min(a_min), max(a_max) {}
        Area(const Coordinates& a_min) : min(a_min), max(a_min) {}

        size_t num_dim() const { return min.size(); }

        void extend(const Coordinates& point)
        {
            for (size_t dim = 0; dim < num_dim(); ++dim) {
                if (point[dim] < min[dim])
                    min[dim] = point[dim];
                if (point[dim] > max[dim])
                    max[dim] = point[dim];
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
            Coordinates min_, max_, current_;

            friend struct Area;
            Iterator(double step, const Coordinates& a_min, const Coordinates& a_max) : step_(step), min_(a_min), max_(a_min), current_(a_min) { set_max(a_max); }
            Iterator() : step_(std::numeric_limits<double>::quiet_NaN()) {}

            void set_max(const Coordinates& a_max)
                {
                    for (auto dim : acmacs::range(max_.size()))
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
            const Coordinates& operator*() const { return current_; }
            const Coordinates* operator->() const { return &current_; }

            const Iterator& operator++()
                {
                    if (current_[0] <= max_[0]) {
                        for (auto dim : acmacs::range(current_.size())) {
                            current_[dim] += step_;
                            if (current_[dim] <= max_[dim]) {
                                std::copy(min_.begin(), min_.begin() + static_cast<Coordinates::difference_type>(dim), current_.begin());
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

    inline std::string to_string(const Area& area, size_t precision = 32) { return to_string(area.min, precision) + ' ' + to_string(area.max, precision); }

    inline std::ostream& operator<<(std::ostream& s, const Area& area) { return s << to_string(area, 4); }

      // ----------------------------------------------------------------------

    class LayoutInterface;

    class LayoutConstIterator
    {
      public:
        Coordinates operator*() const;
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
        const LayoutInterface& parent_;
        mutable size_t point_no_;

        LayoutConstIterator(const LayoutInterface& parent, size_t point_no) : parent_{parent}, point_no_{point_no} {}

        friend class LayoutInterface;

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
        const LayoutInterface& parent_;
        mutable size_t point_no_;
        const size_t dimension_no_;

        LayoutDimensionConstIterator(const LayoutInterface& parent, size_t point_no, size_t dimension_no) : parent_{parent}, point_no_{point_no}, dimension_no_{dimension_no} {}

        friend class LayoutInterface;

    }; // class LayoutDimensionConstIterator

    // ----------------------------------------------------------------------

    class LayoutInterface
    {
      public:
        LayoutInterface() = default;
        LayoutInterface(const LayoutInterface&) = default;
        virtual ~LayoutInterface() = default;
        LayoutInterface& operator=(const LayoutInterface&) = default;

        virtual size_t number_of_points() const noexcept = 0;
        virtual size_t number_of_dimensions() const noexcept = 0;
        virtual const Coordinates operator[](size_t aPointNo) const = 0;
        const Coordinates get(size_t aPointNo) const { return operator[](aPointNo); }
        Coordinates& operator[](size_t) = delete; // use set()!
        virtual double coordinate(size_t aPointNo, size_t aDimensionNo) const = 0;
        double operator()(size_t aPointNo, size_t aDimensionNo) const { return coordinate(aPointNo, aDimensionNo); }
        virtual bool point_has_coordinates(size_t point_no) const = 0;
        virtual std::vector<double> as_flat_vector_double() const = 0;
        virtual std::vector<float> as_flat_vector_float() const = 0;

        double distance(size_t p1, size_t p2, double no_distance = std::numeric_limits<double>::quiet_NaN()) const
        {
            const auto c1 = operator[](p1);
            const auto c2 = operator[](p2);
            return (c1.not_nan() && c2.not_nan()) ? c1.distance(c2) : no_distance;
        }

        // returns indexes for min points for each dimension and max points for each dimension
        virtual std::pair<std::vector<size_t>, std::vector<size_t>> min_max_point_indexes() const;
        // returns boundary coordinates (min and max)
        virtual Area area() const;                                  // for all points
        virtual Area area(const std::vector<size_t>& points) const; // just for the specified point indexes
        virtual LayoutInterface* transform(const Transformation& aTransformation) const;
        virtual Coordinates centroid() const;

        virtual void set(size_t aPointNo, const acmacs::Vector& aCoordinates) = 0;
        virtual void set(size_t point_no, size_t dimension_no, double value) = 0;

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

    }; // class LayoutInterface

    inline std::ostream& operator<<(std::ostream& s, const LayoutInterface& aLayout)
    {
        s << "Layout [";
        for (size_t no = 0; no < aLayout.number_of_points(); ++no)
            s << aLayout[no] << ' ';
        s << ']';
        return s;
    }

    inline Coordinates LayoutConstIterator::operator*() const
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

      // ----------------------------------------------------------------------

    class Layout : public virtual LayoutInterface, public std::vector<double>
    {
      public:
        using Vec = std::vector<double>;

        Layout() = default;
        Layout(const Layout&) = default;
        Layout(size_t aNumberOfPoints, size_t aNumberOfDimensions) : Vec(aNumberOfPoints * aNumberOfDimensions, std::numeric_limits<double>::quiet_NaN()), number_of_dimensions_{aNumberOfDimensions} {}
        Layout(size_t aNumberOfDimensions, const double* first, const double* last) : Vec(first, last), number_of_dimensions_{aNumberOfDimensions} {}
        Layout(const LayoutInterface& aSource) : Vec(aSource.as_flat_vector_double()), number_of_dimensions_{aSource.number_of_dimensions()} {}
        Layout(const LayoutInterface& aSource, const std::vector<size_t>& aIndexes); // make layout by subsetting source
        Layout& operator=(const Layout&) = default;

        size_t number_of_points() const noexcept override { return size() / number_of_dimensions_; }
        size_t number_of_dimensions() const noexcept override { return number_of_dimensions_; }

        void change_number_of_dimensions(size_t num_dim, bool allow_dimensions_increase = false)
        {
            if (!allow_dimensions_increase && num_dim >= number_of_dimensions_) {
                throw std::runtime_error("Layout::change_number_of_dimensions: dimensions increase: " + std::to_string(number_of_dimensions_) + " --> " + std::to_string(num_dim));
                // std::cerr << "WARNING: Layout::change_number_of_dimensions: " << number_of_dimensions_ << " --> " << num_dim << '\n';
            }
            resize(number_of_points() * num_dim);
            number_of_dimensions_ = num_dim;
        }

        const Coordinates operator[](size_t aPointNo) const override
        {
            return {Vec::begin() + static_cast<difference_type>(aPointNo * number_of_dimensions_), Vec::begin() + static_cast<difference_type>((aPointNo + 1) * number_of_dimensions_)};
        }

        double coordinate(size_t aPointNo, size_t aDimensionNo) const override { return at(aPointNo * number_of_dimensions_ + aDimensionNo); }
        bool point_has_coordinates(size_t point_no) const override { return !std::isnan(coordinate(point_no, 0)); }
        std::vector<double> as_flat_vector_double() const override { return *this; }
        std::vector<float> as_flat_vector_float() const override { return {Vec::begin(), Vec::end()}; }

        void set(size_t aPointNo, const acmacs::Vector& aCoordinates) override
        {
            std::copy(aCoordinates.begin(), aCoordinates.end(), Vec::begin() + static_cast<decltype(Vec::begin())::difference_type>(aPointNo * number_of_dimensions()));
        }

        void set_nan(size_t aPointNo)
        {
            const auto first{Vec::begin() + static_cast<difference_type>(aPointNo * number_of_dimensions())}, last{first + static_cast<difference_type>(number_of_dimensions())};
            std::for_each(first, last, [](auto& target) { target = std::numeric_limits<std::decay_t<decltype(target)>>::quiet_NaN(); });
        }

        virtual void set(size_t point_no, size_t dimension_no, double value) override { Vec::operator[](point_no* number_of_dimensions() + dimension_no) = value; }

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

      private:
        size_t number_of_dimensions_;

    }; // class Layout

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
