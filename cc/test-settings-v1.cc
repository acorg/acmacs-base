#include <iostream>

#include "acmacs-base/settings-v1.hh"

// ----------------------------------------------------------------------

struct AAAtPos : public acmacs::settings::v1::object
{
    acmacs::settings::v1::field<size_t>      diverse_index_threshold{this, "diverse_index_threshold", 3};
    acmacs::settings::v1::field<double>      line_length{this, "line_length", 0.5};
    acmacs::settings::v1::field<bool>        report_most_diverse_positions{this, "report_most_diverse_positions", false};
    acmacs::settings::v1::field<std::string> comment{this, "comment"};

    using acmacs::settings::v1::object::object;
};

struct Mod : public acmacs::settings::v1::object
{
    acmacs::settings::v1::field<std::string> name{this, "N"};
    acmacs::settings::v1::field<double>      d1{this, "d1", 1.0/8.0};

    using acmacs::settings::v1::object::object;
};

struct Settings : public acmacs::settings::v1::toplevel
{
    acmacs::settings::v1::field<std::string>       version{this, "  version", "signature-page-settings-v4"};
    acmacs::settings::v1::field_object<AAAtPos>    aa_at_pos{this, "aa_at_pos"};
    acmacs::settings::v1::field_array<double>      viewport{this, "viewport", {0.125, 0.25, 122}};
    acmacs::settings::v1::field_array_of<Mod>      mods{this, "mods"};
    acmacs::settings::v1::field<acmacs::Size>      size{this, "size", {7, 8}};
    acmacs::settings::v1::field<acmacs::Offset>    offset{this, "offset", {-1, 111}};
    acmacs::settings::v1::field<Color>             fill{this, "fill", "cornflowerblue"};
    acmacs::settings::v1::field<acmacs::TextStyle> text_style{this, "text_style", {"monospace"}};
};

// ----------------------------------------------------------------------

int main()
{
    int exit_code = 0;

    try {
        Settings s1;
        s1.inject_default();
        std::cout << s1.pretty() << '\n';

        std::cout << "report_most_diverse_positions: " << s1.aa_at_pos->report_most_diverse_positions << '\n';
        s1.aa_at_pos->report_most_diverse_positions = true;
        std::cout << "report_most_diverse_positions: " << s1.aa_at_pos->report_most_diverse_positions << '\n';

        std::cout << "line_length: " << s1.aa_at_pos->line_length << '\n';
        s1.aa_at_pos->line_length = 1.2;
        std::cout << "line_length: " << s1.aa_at_pos->line_length << '\n';
        std::cout << "size: " << s1.size << '\n';
        s1.size = acmacs::Size{9, 10};
        std::cout << "size: " << s1.size << '\n';
        std::cout << "offset: " << s1.offset << '\n';
        s1.offset = acmacs::Offset{12, -13.5};
        std::cout << "offset: " << s1.offset << '\n';
        std::cout << "fill: \"" << s1.fill << "\" " << Color(s1.fill).to_hex_string() << '\n';
        s1.fill = Color("#123456");
        std::cout << "fill: \"" << s1.fill << "\" " << Color(s1.fill).to_hex_string() <<  '\n';
        std::cout << "text_style: " << s1.text_style << '\n';
        s1.text_style = acmacs::TextStyle("serif");
        std::cout << "text_style: " << s1.text_style << '\n';

        std::cout << "aa_at_pos: " << s1.aa_at_pos << '\n';
        std::cout << "viewport: " << s1.viewport << '\n';
        s1.viewport.append(s1.viewport[2]);
        std::cout << "viewport: " << s1.viewport << '\n';

        std::cout << '\n';
        auto mod1 = s1.mods.append();
        mod1->name = "first";
        s1.mods.append()->name = "second";
        std::cout << s1.pretty() << '\n';
        if (auto found = s1.mods.find_if([](const Mod& mod) { return mod.name == "second"; }); found)
            std::cout << "found: " << *found << '\n';
        else
            std::cout << "not found!\n";
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 1;
    }

    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
