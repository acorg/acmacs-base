#pragma once

#include "acmacs-base/settings-v3.hh"
#include "acmacs-base/settings-v3-env.hh"

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
                AD_INFO("TestData \"{}\"\n{}", name, environment().format("    "sv));
            }
            return true;
        }
    };

} // namespace acmacs::settings::v3

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
