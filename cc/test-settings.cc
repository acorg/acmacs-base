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

struct Settings : public acmacs::settings::toplevel
{
    acmacs::settings::field<std::string>    version{this, "version", "signature-page-settings-v4"};
    acmacs::settings::field_object<AAAtPos> aa_at_pos{this, "aa_at_pos"};
};

// ----------------------------------------------------------------------

int main()
{
    int exit_code = 0;

    try {
        Settings s1;
        s1.inject_default();
        s1.write_to_file("-");
        std::cout << '\n';

        std::cout << "report_most_diverse_positions: " << s1.aa_at_pos->report_most_diverse_positions << '\n';
        s1.aa_at_pos->report_most_diverse_positions = true;
        std::cout << "report_most_diverse_positions: " << s1.aa_at_pos->report_most_diverse_positions << '\n';

        std::cout << "line_length: " << s1.aa_at_pos->line_length << '\n';
        s1.aa_at_pos->line_length = 1.2;
        std::cout << "line_length: " << s1.aa_at_pos->line_length << '\n';

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
