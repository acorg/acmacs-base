#pragma once

#include "acmacs-base/named-type.hh"

// ----------------------------------------------------------------------

namespace acmacs::html::inline v1
{
    using raw = named_string_t<struct raw_tag>;

    class Generator
    {
      public:
        std::string generate() const;

        void add_css_link(std::string_view href);
        void add_css(std::string_view data);

        void add_script_link(std::string_view src);
        void add_script(std::string_view data);
        void add_is_safari(std::string_view var_name = "isSafari");

        void title(std::string_view title);

        void add_to_body(std::string_view data);

      private:
        std::vector<std::string> head_fields_;
        std::vector<std::string> body_fields_;
    };

} // namespace acmacs::html::inline v1

// ----------------------------------------------------------------------
