#include "acmacs-base/layout.hh"

// ----------------------------------------------------------------------

std::pair<std::vector<size_t>, std::vector<size_t>> acmacs::LayoutInterface::min_max_point_indexes() const
{
    const auto num_dim = number_of_dimensions();
    std::vector<size_t> min_points(num_dim, 0), max_points(num_dim, 0);
    size_t point_no = 0;
    for (; !operator[](point_no).not_nan(); ++point_no); // skip NaN points at the beginning
    Coordinates min_coordinates(operator[](point_no));
    Coordinates max_coordinates(min_coordinates);
    ++point_no;
    for (; point_no < number_of_points(); ++point_no) {
        const auto point = operator[](point_no);
        if (point.not_nan()) {
            for (size_t dim = 0; dim < num_dim; ++dim) {
                if (point[dim] < min_coordinates[dim]) {
                    min_coordinates[dim] = point[dim];
                    min_points[dim] = point_no;
                }
                if (point[dim] > max_coordinates[dim]) {
                    max_coordinates[dim] = point[dim];
                    max_points[dim] = point_no;
                }
            }
        }
    }
    return {min_points, max_points};

} // acmacs::LayoutInterface::min_max_point_indexes

// ----------------------------------------------------------------------

acmacs::Area acmacs::LayoutInterface::area() const
{
    size_t point_no = 0;
    for (; !operator[](point_no).not_nan(); ++point_no); // skip NaN points at the beginning
    Area result(operator[](point_no));
    ++point_no;
    for (; point_no < number_of_points(); ++point_no) {
        if (const auto point = operator[](point_no); point.not_nan())
            result.extend(point);
    }
    return result;

} // acmacs::LayoutInterface::boundaries

// ----------------------------------------------------------------------

acmacs::Area acmacs::LayoutInterface::area(const std::vector<size_t>& points) const // just for the specified point indexes
{
    Area result(operator[](points.front()));
    for (auto point_no : points) {
        if (const auto point = operator[](point_no); point.not_nan())
            result.extend(point);
    }
    return result;

} // acmacs::LayoutInterface::boundaries

// ----------------------------------------------------------------------

acmacs::LayoutInterface* acmacs::LayoutInterface::transform(const acmacs::Transformation& aTransformation) const
{
    auto* result = new acmacs::Layout(number_of_points(), number_of_dimensions());
    for (size_t p_no = 0; p_no < number_of_points(); ++p_no)
        result->set(p_no, get(p_no).transform(aTransformation));
    return result;

} // acmacs::LayoutInterface::transform

// ----------------------------------------------------------------------

acmacs::Coordinates acmacs::LayoutInterface::centroid() const
{
    Coordinates result(number_of_dimensions(), 0.0);
    size_t num_non_nan = number_of_points();
    for (size_t p_no = 0; p_no < number_of_points(); ++p_no) {
        const auto coord = get(p_no);
        if (coord.not_nan())
            result += coord;
        else
            --num_non_nan;
    }
    result /= num_non_nan;
    return result;

} // acmacs::LayoutInterface::centroid

// ----------------------------------------------------------------------

acmacs::Layout::Layout(const LayoutInterface& aSource, const std::vector<size_t>& aIndexes)
    : std::vector<double>(aIndexes.size() * aSource.number_of_dimensions(), std::numeric_limits<double>::quiet_NaN()),
    number_of_dimensions_{aSource.number_of_dimensions()}
{
    auto target = begin();
    for (auto index : aIndexes) {
        const auto coord{aSource[index]};
        std::copy(coord.begin(), coord.end(), target);
        target += static_cast<decltype(target)::difference_type>(number_of_dimensions_);
    }

} // acmacs::Layout::Layout

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
