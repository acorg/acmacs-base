#pragma once

#include <variant>
#include <functional>           // std::reference_wrapper
#include <iostream>
#include <fstream>

// ----------------------------------------------------------------------

namespace acmacs::file
{
    class ifstream
    {
     public:
        ifstream(std::string_view filename) : backend_{std::cin}
            {
                if (filename != "-")
                    backend_ = std::ifstream(std::string{filename});
            }
        // ifstream(std::string_view filename) : ifstream(std::string(filename)) {}

        std::istream& stream() { return std::visit([](auto&& stream) -> std::istream& { return stream; }, backend_); }
        const std::istream& stream() const { return std::visit([](auto&& stream) -> const std::istream& { return stream; }, backend_); }

        std::istream& operator*() { return stream(); }
        std::istream* operator->() { return &stream(); }
        operator std::istream&() { return stream(); }
        std::string read() { return {std::istreambuf_iterator<char>(stream()), {}}; }
        explicit operator bool() const { return bool(stream()); }

     private:
        std::variant<std::reference_wrapper<std::istream>,std::ifstream> backend_;
    };

    class ofstream
    {
     public:
        ofstream(std::string_view filename) : backend_{std::cout}
            {
                if (filename == "=")
                    backend_ = std::cerr;
                else if (filename != "-")
                    backend_ = std::ofstream(std::string{filename});
            }

        std::ostream& stream() { return std::visit([](auto&& stream) -> std::ostream& { return stream; }, backend_); }
        const std::ostream& stream() const { return std::visit([](auto&& stream) -> const std::ostream& { return stream; }, backend_); }

        std::ostream& operator*() { return stream(); }
        std::ostream* operator->() { return &stream(); }
        operator std::ostream&() { return stream(); }
        explicit operator bool() const { return bool(stream()); }

     private:
        std::variant<std::reference_wrapper<std::ostream>,std::ofstream> backend_;
    };

}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
