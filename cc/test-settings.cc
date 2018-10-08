#include <iostream>

#include "acmacs-base/settings.hh"

// ----------------------------------------------------------------------

struct AAAtPos : public acmacs::settings::object
{
    acmacs::settings::field<size_t>      diverse_index_threshold{this, "diverse_index_threshold", 3};
    acmacs::settings::field<double>      line_length{this, "line_length", 0.5};
    acmacs::settings::field<bool>        report_most_diverse_positions{this, "report_most_diverse_positions", false};
    acmacs::settings::field<std::string> comment{this, "comment"};

    using acmacs::settings::object::object;
};

struct Mod : public acmacs::settings::object
{
    acmacs::settings::field<std::string> name{this, "N"};
    acmacs::settings::field<double>      d1{this, "d1", 1.0/8.0};

    using acmacs::settings::object::object;
};

struct Settings : public acmacs::settings::toplevel
{
    acmacs::settings::field<std::string>    version{this, "  version", "signature-page-settings-v4"};
    acmacs::settings::field_object<AAAtPos> aa_at_pos{this, "aa_at_pos"};
    acmacs::settings::field_array<double>   viewport{this, "viewport", {0.125, 0.25, 122}};
    acmacs::settings::field_array_of<Mod>   mods{this, "mods"};
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
