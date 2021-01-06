#pragma once

#include <string>
#include <string_view>

// ----------------------------------------------------------------------

namespace acmacs::file
{
    class temp
    {
     public:
        temp(std::string prefix, std::string suffix, bool autoremove = true);
        temp(std::string suffix, bool autoremove = true);
        ~temp();

        temp& operator = (temp&& aFrom) noexcept { name = std::move(aFrom.name); fd = aFrom.fd; aFrom.name.clear(); return *this; }
        operator std::string() const { return name; }
        operator std::string_view() const { return name; }
        constexpr operator int() const { return fd; }

     private:
        std::string name;
        bool autoremove_;
        int fd;

        std::string make_template(std::string prefix);

    }; // class temp
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
