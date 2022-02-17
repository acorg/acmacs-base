#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

// ----------------------------------------------------------------------

namespace acmacs::file
{
    enum class force_compression { no, yes };
    enum class backup_file { no, yes };
    enum class backup_move { no, yes };

      // ----------------------------------------------------------------------

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
        read_access(std::string_view aFilename);
        ~read_access();
        read_access(const read_access&) = delete;
        read_access(read_access&&);
        read_access& operator=(const read_access&) = delete;
        read_access& operator=(read_access&&);
        operator std::string() const { return mapped_ ? decompress_if_necessary({mapped_, len_}) : decompress_if_necessary(data_); }
        size_t size() const { return mapped_ ? len_ : data_.size(); }
        const char* data() const { return mapped_ ? mapped_ : data_.data(); }
        std::string_view raw() const { return mapped_ ? std::string_view(mapped_, len_) : std::string_view{data_}; }
        bool valid() const { return mapped_ != nullptr || !data_.empty(); }

     private:
        int fd = -1;
        size_t len_ = 0;
        char* mapped_ = nullptr;
        std::string data_;

    }; // class read_access

    inline read_access read(std::string_view aFilename) { return read_access{aFilename}; }
    std::string read_from_file_descriptor(int fd, size_t chunk_size = 1024);
    inline std::string read_stdin() { return read_from_file_descriptor(0); }
    void write(std::string_view aFilename, std::string_view aData, force_compression aForceCompression = force_compression::no, backup_file aBackupFile = backup_file::yes);

    void backup(std::string_view to_backup, std::string_view backup_dir, backup_move bm = backup_move::no);
    void backup(std::string_view to_backup, backup_move bm = backup_move::no);

} // namespace acmacs::file

// ----------------------------------------------------------------------
