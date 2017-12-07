#include "acmacs-base/layout.hh"

// ----------------------------------------------------------------------

std::pair<std::vector<size_t>, std::vector<size_t>> acmacs::LayoutInterface::min_max_point_indexes() const
{
    const auto num_dim = number_of_dimensions();
    std::vector<size_t> min_points(num_dim, 0), max_points(num_dim, 0);
    Coordinates min_coordinates(operator[](0));
    Coordinates max_coordinates(min_coordinates);
    for (size_t point_no = 1; point_no < number_of_points(); ++point_no) {
        const auto point = operator[](point_no);
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
    return {min_points, max_points};

} // acmacs::LayoutInterface::min_max_point_indexes

// ----------------------------------------------------------------------

std::pair<acmacs::Coordinates, acmacs::Coordinates> acmacs::LayoutInterface::boundaries() const
{
    const auto num_dim = number_of_dimensions();
    Coordinates min_coordinates(operator[](0));
    Coordinates max_coordinates(min_coordinates);
    for (size_t point_no = 1; point_no < number_of_points(); ++point_no) {
        const auto point = operator[](point_no);
        for (size_t dim = 0; dim < num_dim; ++dim) {
            if (point[dim] < min_coordinates[dim]) {
                min_coordinates[dim] = point[dim];
            }
            if (point[dim] > max_coordinates[dim]) {
                max_coordinates[dim] = point[dim];
            }
        }
    }
    return {min_coordinates, max_coordinates};

} // acmacs::LayoutInterface::boundaries

// ----------------------------------------------------------------------

acmacs::LayoutInterface* acmacs::LayoutInterface::transform(const acmacs::Transformation& aTransformation) const
{
    auto* result = new acmacs::Layout(number_of_points());
    for (size_t p_no = 0; p_no < number_of_points(); ++p_no)
        result->set(p_no, get(p_no).transform(aTransformation));
    return result;

} // acmacs::LayoutInterface::transform

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
