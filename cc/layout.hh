#pragma once

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

        inline Coordinates transform(const Transformation& aTransformation) const
            {
                const auto [x, y] = aTransformation.transform(operator[](0), operator[](1));
                return {x, y};
            }

        inline bool not_nan() const
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
        inline LayoutInterface() = default;
        virtual ~LayoutInterface() = default;

        virtual size_t number_of_points() const noexcept = 0;
        virtual size_t number_of_dimensions() const noexcept = 0;
        virtual const Coordinates operator[](size_t aPointNo) const = 0;
        inline const Coordinates get(size_t aPointNo) const { return operator[](aPointNo); }
        Coordinates& operator[](size_t) = delete; // use set()!
        virtual double coordinate(size_t aPointNo, size_t aDimensionNo) const = 0;
        virtual void set(size_t aPointNo, const Coordinates& aCoordinates) = 0;

        inline double distance(size_t p1, size_t p2, double no_distance = std::numeric_limits<double>::quiet_NaN()) const
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

    class Layout : public virtual LayoutInterface, public std::vector<Coordinates>
    {
     public:
        inline Layout() = default;
        inline Layout(size_t aNumberOfPoints) : std::vector<Coordinates>(aNumberOfPoints) {}
        Layout(const LayoutInterface& aSource, const std::vector<size_t>& aIndexes); // make layout bu subsetting source

        inline size_t number_of_points() const noexcept override { return size(); }

        inline size_t number_of_dimensions() const noexcept override
            {
                for (const auto& point: *this) {
                    if (!point.empty())
                        return point.size();
                }
                return 0;
            }

        inline const Coordinates operator[](size_t aPointNo) const override { return at(aPointNo); }
        inline double coordinate(size_t aPointNo, size_t aDimensionNo) const override { return at(aPointNo).at(aDimensionNo); }
        inline void set(size_t aPointNo, const Coordinates& aCoordinates) override { at(aPointNo) = aCoordinates; }

    }; // class Layout

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
