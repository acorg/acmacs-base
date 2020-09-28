#pragma once

#include "acmacs-base/settings-v3.hh"
#include "acmacs-base/settings-v3-env.hh"
#include "acmacs-base/point-style.hh"

// ----------------------------------------------------------------------

namespace acmacs::settings::v3
{
    class TestData : public Data
    {
      protected:
        bool apply_built_in(std::string_view name) override
        {
            if (!Data::apply_built_in(name)) {
                using namespace std::string_view_literals;
                if (name == "test-antigens"sv)
                    apply_test_antigens();
                else
                    AD_INFO("TestData \"{}\"\n{}", name, environment().format("    "sv));
            }
            return true;
        }

      private:
        void apply_test_antigens()
        {
            using namespace std::string_view_literals;
            acmacs::PointStyleModified style;
            rjson::v3::set_if_not_null<Pixels>(environment().get("size"sv), [&style](Pixels sz) { style.size(sz); });
            rjson::v3::set_if_not_null<const acmacs::color::Modifier&>(environment().get("fill"sv), [&style](const acmacs::color::Modifier& fill) { style.fill(fill); });
            rjson::v3::set_if_not_null<const acmacs::color::Modifier&>(environment().get("outline"sv), [&style](const acmacs::color::Modifier& outline) { style.outline(outline); });
            AD_INFO("apply_test_antigens {}", style);
        }
    };

} // namespace acmacs::settings::v3

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
