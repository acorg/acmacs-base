#include "acmacs-base/layout.hh"

// ----------------------------------------------------------------------

std::pair<std::vector<size_t>, std::vector<size_t>> acmacs::Layout::min_max_point_indexes() const
{
    const auto num_dim = number_of_dimensions();
    std::vector<size_t> min_points(*num_dim, 0), max_points(*num_dim, 0);
    size_t point_no = 0;
    for (; !operator[](point_no).exists(); ++point_no); // skip NaN points at the beginning
    PointCoordinates min_coordinates(operator[](point_no));
    PointCoordinates max_coordinates(min_coordinates);
    ++point_no;
    for (; point_no < number_of_points(); ++point_no) {
        const auto point = operator[](point_no);
        if (point.exists()) {
            for (auto dim : range(num_dim)) {
                if (point[dim] < min_coordinates[dim]) {
                    min_coordinates[dim] = point[dim];
                    min_points[*dim] = point_no;
                }
                if (point[dim] > max_coordinates[dim]) {
                    max_coordinates[dim] = point[dim];
                    max_points[*dim] = point_no;
                }
            }
        }
    }
    return {min_points, max_points};

} // acmacs::Layout::min_max_point_indexes

// ----------------------------------------------------------------------

acmacs::Area acmacs::Layout::area() const
{
    size_t point_no = 0;
    for (; !operator[](point_no).exists(); ++point_no); // skip NaN points at the beginning
    Area result(operator[](point_no));
    ++point_no;
    for (; point_no < number_of_points(); ++point_no) {
        if (const auto point = operator[](point_no); point.exists())
            result.extend(point);
    }
    return result;

} // acmacs::Layout::boundaries

// ----------------------------------------------------------------------

acmacs::Area acmacs::Layout::area(const std::vector<size_t>& points) const // just for the specified point indexes
{
    Area result(operator[](points.front()));
    for (auto point_no : points) {
        if (const auto point = operator[](point_no); point.exists())
            result.extend(point);
    }
    return result;

} // acmacs::Layout::boundaries

// ----------------------------------------------------------------------

std::shared_ptr<acmacs::Layout> acmacs::Layout::transform(const acmacs::Transformation& aTransformation) const
{
    auto result = std::make_shared<acmacs::Layout>(number_of_points(), number_of_dimensions());
    for (size_t p_no = 0; p_no < number_of_points(); ++p_no)
        result->update(p_no, aTransformation.transform(at(p_no)));
    return result;

} // acmacs::Layout::transform

// ----------------------------------------------------------------------

acmacs::PointCoordinates acmacs::Layout::centroid() const
{
    PointCoordinates result(static_cast<double>(*number_of_dimensions()), 0.0);
    size_t num_non_nan = number_of_points();
    for (size_t p_no = 0; p_no < number_of_points(); ++p_no) {
        if (const auto coord = at(p_no); coord.exists())
            result += coord;
        else
            --num_non_nan;
    }
    result /= static_cast<double>(num_non_nan);
    return result;

} // acmacs::Layout::centroid

// ----------------------------------------------------------------------

acmacs::Layout::Layout(const Layout& source, const std::vector<size_t>& indexes)
    : Vec(indexes.size() * source.number_of_dimensions().get(), std::numeric_limits<double>::quiet_NaN()), number_of_dimensions_{source.number_of_dimensions()}
{
    auto target = Vec::begin();
    for (auto index : indexes) {
        const auto coord{source[index]};
        std::copy(coord.begin(), coord.end(), target);
        target += static_cast<decltype(target)::difference_type>(*number_of_dimensions_);
    }

} // acmacs::Layout::Layout

// ----------------------------------------------------------------------

std::vector<std::pair<double, double>> acmacs::Layout::minmax() const
{
    std::vector<std::pair<double, double>> result(*number_of_dimensions_);
    // using diff_t = decltype(result)::difference_type;
    // Layout may contain NaNs (disconnected points), avoid them when finding minmax
    auto it = Vec::begin();
    while (it != Vec::end()) {
        number_of_dimensions_t valid_dims{0};
        for (auto dim : range(number_of_dimensions_)) {
            if (!std::isnan(*it)) {
                result[*dim] = std::pair(*it, *it);
                ++valid_dims;
            }
            ++it;
        }
        if (valid_dims == number_of_dimensions_)
            break;
    }
    while (it != Vec::end()) {
        for (auto dim : range(number_of_dimensions_)) {
            if (!std::isnan(*it)) {
                result[*dim].first = std::min(result[*dim].first, *it);
                result[*dim].second = std::max(result[*dim].second, *it);
            }
            ++it;
        }
    }
    return result;

} // acmacs::Layout::minmax

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
