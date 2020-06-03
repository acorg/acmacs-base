#include "acmacs-base/html.hh"
#include "acmacs-base/string-join.hh"
#include "acmacs-base/string.hh"

// ----------------------------------------------------------------------

constexpr static std::string_view sPattern{R"(<!DOCTYPE html>
<html>
 <head>
  <meta charset="utf-8" />
{head}
 </head>
 <body>
{body}
 </body>
</html>
)"};

constexpr size_t sIndent{1};

// ----------------------------------------------------------------------

std::string acmacs::html::v1::Generator::generate() const
{
    const auto make_content = [](const auto& rows, size_t indent) {
        const std::string prefix(indent, ' ');
        return prefix + ::string::replace(acmacs::string::join(acmacs::string::join_newline, std::begin(rows), std::end(rows)), "\n", fmt::format("\n{}", prefix), fmt::format("\n{}\n", prefix), "\n\n");
    };

    return fmt::format(sPattern,
                       fmt::arg("head", make_content(head_fields_, sIndent * 2)),
                       fmt::arg("body", make_content(body_fields_, sIndent * 2))
                       );

} // acmacs::html::v1::Generator::generate

// ----------------------------------------------------------------------

void acmacs::html::v1::Generator::add_css_link(std::string_view href)
{
    head_fields_.push_back(fmt::format(R"(<link href="{href}" rel="stylesheet">)", fmt::arg("href", href)));

} // acmacs::html::v1::Generator::add_css_link

// ----------------------------------------------------------------------

void acmacs::html::v1::Generator::add_css(std::string_view data)
{
    head_fields_.push_back(fmt::format("<style>\n{data}\n</style>", fmt::arg("data", data)));

} // acmacs::html::v1::Generator::add_css

// ----------------------------------------------------------------------

void acmacs::html::v1::Generator::add_script_link(std::string_view src)
{
    head_fields_.push_back(fmt::format(R"(<script src="{src}"></script>)", fmt::arg("src", src)));

} // acmacs::html::v1::Generator::add_script_link

// ----------------------------------------------------------------------

void acmacs::html::v1::Generator::add_script(std::string_view data)
{
    head_fields_.push_back(fmt::format("<script>\n{data}\n</script>", fmt::arg("data", data)));

} // acmacs::html::v1::Generator::add_script

// ----------------------------------------------------------------------

void acmacs::html::v1::Generator::add_is_safari(std::string_view var_name)
{
    head_fields_.push_back(fmt::format("<script>const {var_name} = /^((?!chrome|android).)*safari/i.test(navigator.userAgent);</script>", fmt::arg("var_name", var_name)));

} // acmacs::html::v1::Generator::add_is_safari

// ----------------------------------------------------------------------

void acmacs::html::v1::Generator::title(std::string_view title)
{
    head_fields_.push_back(fmt::format("<title>{title}</title>", fmt::arg("title", title)));

} // acmacs::html::v1::Generator::title

// ----------------------------------------------------------------------

void acmacs::html::v1::Generator::add_to_body(std::string_view data)
{
    body_fields_.emplace_back(data);

} // acmacs::html::v1::Generator::add_to_body

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
