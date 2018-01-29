#pragma once

#include <limits>
#include <cassert>

#include "acmacs-base/vector.hh"
#include "acmacs-base/transformation.hh"
#include "acmacs-base/stream.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    class Coordinates : public Vector
    {
     public:
        using Vector::Vector;

        Coordinates transform(const Transformation& aTransformation) const
            {
                const auto [x, y] = aTransformation.transform(operator[](0), operator[](1));
                return {x, y};
            }

        bool not_nan() const
            {
                return !empty() && std::all_of(begin(), end(), [](double value) -> bool { return ! std::isnan(value); });
            }

    }; // class Coordinates

    inline std::ostream& operator<<(std::ostream& s, const Coordinates& c)
    {
        stream_internal::write_to_stream(s, c, "[", "]", ", ");
        return s;
    }

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
        virtual std::pair<Coordinates, Coordinates> boundaries() const;
        virtual LayoutInterface* transform(const Transformation& aTransformation) const;
        virtual Coordinates centroid() const;

        virtual void set(size_t aPointNo, const Coordinates& aCoordinates) = 0;
        virtual void set(size_t point_no, size_t dimension_no, double value) = 0;

    }; // class LayoutInterface

    inline std::ostream& operator<<(std::ostream& s, const LayoutInterface& aLayout)
    {
        s << "Layout [";
        for (size_t no = 0; no < aLayout.number_of_points(); ++no)
            s << aLayout[no] << ' ';
        s << ']';
        return s;
    }

// ----------------------------------------------------------------------

    class Layout : public virtual LayoutInterface, public std::vector<double>
    {
     public:
        Layout() = default;
        Layout(size_t aNumberOfPoints, size_t aNumberOfDimensions)
            : std::vector<double>(aNumberOfPoints * aNumberOfDimensions, std::numeric_limits<double>::quiet_NaN()),
            number_of_dimensions_{aNumberOfDimensions}
            {}
        Layout(const Layout&) = default;
        Layout(const LayoutInterface& aSource) : std::vector<double>(aSource.as_flat_vector_double()), number_of_dimensions_{aSource.number_of_dimensions()} {}
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
                using diff_t = decltype(begin())::difference_type;
                return {begin() + static_cast<diff_t>(aPointNo * number_of_dimensions_), begin() + static_cast<diff_t>((aPointNo + 1) * number_of_dimensions_)};
            }

        double coordinate(size_t aPointNo, size_t aDimensionNo) const override { return at(aPointNo * number_of_dimensions_ + aDimensionNo); }
        bool point_has_coordinates(size_t point_no) const override { return !std::isnan(coordinate(point_no, 0)); }
        std::vector<double> as_flat_vector_double() const override { return *this; }
        std::vector<float> as_flat_vector_float() const override { return {begin(), end()}; }
        void set(size_t aPointNo, const Coordinates& aCoordinates) override
            {
                std::copy(aCoordinates.begin(), aCoordinates.end(), begin() + static_cast<decltype(begin())::difference_type>(aPointNo * number_of_dimensions()));
            }

        virtual void set(size_t point_no, size_t dimension_no, double value) override { std::vector<double>::operator[](point_no * number_of_dimensions() + dimension_no) = value; }

     private:
        size_t number_of_dimensions_;

    }; // class Layout

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
