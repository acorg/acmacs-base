#pragma once

#include <string>

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
        inline read_access() = default;
        read_access(std::string aFilename);
        ~read_access();
        read_access(const read_access&) = delete;
        read_access(read_access&&);
        read_access& operator=(const read_access&) = delete;
        read_access& operator=(read_access&&);
        inline operator std::string() const { return decompress_if_necessary({mapped, len}); }
        inline size_t size() const { return len; }
        inline const char* data() const { return mapped; }
        inline bool valid() const { return mapped != nullptr; }

     private:
        int fd = -1;
        size_t len = 0;
        char* mapped = nullptr;

    }; // class read_access

    inline read_access read(std::string aFilename) { return {aFilename}; }
    std::string read_from_file_descriptor(int fd, size_t chunk_size = 1024);
    inline std::string read_stdin() { return read_from_file_descriptor(0); }
    void write(std::string aFilename, std::string_view aData, ForceCompression aForceCompression = ForceCompression::No, BackupFile aBackupFile = BackupFile::Yes);
    void backup(std::string aFilename);

      // ----------------------------------------------------------------------

    class temp
    {
     public:
        temp(std::string suffix);
        ~temp();

        inline temp& operator = (temp&& aFrom) noexcept { name = std::move(aFrom.name); fd = aFrom.fd; aFrom.name.clear(); return *this; }
        inline operator std::string() const { return name; }
        inline operator int() const { return fd; }

     private:
        std::string name;
        int fd;

        std::string make_template();

    }; // class temp

} // namespace acmacs::file

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
