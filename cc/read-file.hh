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

      // ----------------------------------------------------------------------

    std::string read(std::string aFilename, bool throw_if_absent = false);
    std::string read_from_file_descriptor(int fd, size_t chunk_size = 1024);
    inline std::string read_stdin() { return read_from_file_descriptor(0); }
    void write(std::string aFilename, std::string aData, ForceCompression aForceCompression = ForceCompression::No, BackupFile aBackupFile = BackupFile::Yes);
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
