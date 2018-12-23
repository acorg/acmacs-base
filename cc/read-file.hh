#pragma once

#include <string>
#include <variant>
#include <functional>
#include <iostream>
#include <fstream>

#include "acmacs-base/filesystem.hh"

// ----------------------------------------------------------------------

namespace acmacs::file
{
    enum class ForceCompression { No, Yes };
    enum class BackupFile { No, Yes };

      // ----------------------------------------------------------------------

    inline bool exists(std::string aFilename) { return fs::exists(aFilename); }

    std::string decompress_if_necessary(std::string_view aSource);

      // ----------------------------------------------------------------------

    class file_error : public std::runtime_error { public: using std::runtime_error::runtime_error; };
    class not_opened : public file_error { public: not_opened(std::string aMsg) : file_error("cannot open " + aMsg) {} };
    class cannot_read : public file_error { public: cannot_read(std::string aMsg) : file_error("cannot read " + aMsg) {} };
    class not_found : public file_error { public: not_found(std::string aFilename) : file_error("not found: " + aFilename) {} };

    class read_access
    {
     public:
        read_access() = default;
        read_access(std::string aFilename);
        ~read_access();
        read_access(const read_access&) = delete;
        read_access(read_access&&);
        read_access& operator=(const read_access&) = delete;
        read_access& operator=(read_access&&);
        operator std::string() const { return decompress_if_necessary({mapped, len}); }
        size_t size() const { return len; }
        const char* data() const { return mapped; }
        bool valid() const { return mapped != nullptr; }

     private:
        int fd = -1;
        size_t len = 0;
        char* mapped = nullptr;

    }; // class read_access

    inline read_access read(std::string aFilename) { return {aFilename}; }
    std::string read_from_file_descriptor(int fd, size_t chunk_size = 1024);
    inline std::string read_stdin() { return read_from_file_descriptor(0); }
    void write(std::string aFilename, std::string_view aData, ForceCompression aForceCompression = ForceCompression::No, BackupFile aBackupFile = BackupFile::Yes);
    inline void write(std::string_view aFilename, std::string_view aData, ForceCompression aForceCompression = ForceCompression::No, BackupFile aBackupFile = BackupFile::Yes) { write(std::string(aFilename), aData, aForceCompression, aBackupFile); }
    void backup(const fs::path& to_backup, const fs::path& backup_dir);
    inline void backup(const fs::path& to_backup) { backup(to_backup, to_backup.parent_path() / ".backup"); }

      // ----------------------------------------------------------------------

    class temp
    {
     public:
        temp(std::string prefix, std::string suffix);
        temp(std::string suffix);
        ~temp();

        inline temp& operator = (temp&& aFrom) noexcept { name = std::move(aFrom.name); fd = aFrom.fd; aFrom.name.clear(); return *this; }
        inline operator std::string() const { return name; }
        inline operator int() const { return fd; }

     private:
        std::string name;
        int fd;

        std::string make_template(std::string prefix);

    }; // class temp

    class ifstream
    {
     public:
        ifstream(std::string filename) : backend_(std::cin)
            {
                if (filename != "-")
                    backend_ = std::ifstream(filename);
            }
        ifstream(std::string_view filename) : ifstream(std::string(filename)) {}

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
        ofstream(std::string filename) : backend_(std::cout)
            {
                if (filename == "=")
                    backend_ = std::cerr;
                else if (filename != "-")
                    backend_ = std::ofstream(filename);
            }
        ofstream(std::string_view filename) : ofstream(std::string(filename)) {}

        std::ostream& stream() { return std::visit([](auto&& stream) -> std::ostream& { return stream; }, backend_); }
        const std::ostream& stream() const { return std::visit([](auto&& stream) -> const std::ostream& { return stream; }, backend_); }

        std::ostream& operator*() { return stream(); }
        std::ostream* operator->() { return &stream(); }
        operator std::ostream&() { return stream(); }
        explicit operator bool() const { return bool(stream()); }

     private:
        std::variant<std::reference_wrapper<std::ostream>,std::ofstream> backend_;
    };

} // namespace acmacs::file

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
