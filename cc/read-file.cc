#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "acmacs-base/xz.hh"
#include "acmacs-base/bzip2.hh"
#include "acmacs-base/read-file.hh"

// ----------------------------------------------------------------------

std::string acmacs::file::read(std::string aFilename, bool throw_if_absent)
{
    std::string buffer;
    if (fs::exists(aFilename)) {
        const auto size = fs::file_size(aFilename);
        int f = ::open(aFilename.c_str(), O_RDONLY);
        if (f >= 0) {
            buffer.resize(size, ' '); // reserve space
            if (::read(f, &*buffer.begin(), size) < 0)
                throw std::runtime_error(std::string("Cannot read ") + aFilename + ": " + strerror(errno));
            close(f);
        }
        else {
            throw std::runtime_error(std::string("Cannot open ") + aFilename + ": " + strerror(errno));
        }
    }
    else if (throw_if_absent) {
        throw std::runtime_error{"file not found: " + aFilename};
    }
    else {
        buffer = aFilename;
    }
    if (xz_compressed(buffer))
        buffer = xz_decompress(buffer);
    else if (bz2_compressed(buffer))
        buffer = bz2_decompress(buffer);
    return buffer;

} // acmacs::file::read

// ----------------------------------------------------------------------

std::string acmacs::file::read_from_file_descriptor(int fd, size_t chunk_size)
{
    std::string buffer;
    std::string::size_type offset = 0;
    for (;;) {
        buffer.resize(buffer.size() + chunk_size, ' ');
        const auto bytes_read = ::read(fd, (&*buffer.begin()) + offset, chunk_size);
        if (bytes_read < 0)
            throw std::runtime_error(std::string("Cannot read from file descriptor: ") + strerror(errno));
        if (static_cast<size_t>(bytes_read) < chunk_size) {
            buffer.resize(buffer.size() - chunk_size + static_cast<size_t>(bytes_read));
            break;
        }
        offset += static_cast<size_t>(bytes_read);
    }
    if (xz_compressed(buffer))
        buffer = xz_decompress(buffer);
    return buffer;

} // acmacs::file::read_from_file_descriptor

// ----------------------------------------------------------------------

void acmacs::file::backup(std::string aFilename)
{
    fs::path to_backup{aFilename};
    if (fs::exists(to_backup)) {
        fs::path backup_dir = to_backup.parent_path() / ".backup";
        create_directory(backup_dir);

        for (int version = 1; version < 1000; ++version) {
            char infix[10];
            std::sprintf(infix, ".~%03d~", version);
            fs::path new_name = backup_dir / (to_backup.stem().string() + infix + to_backup.extension().string());
            if (!fs::exists(new_name) || version == 999) {
                fs::copy_file(to_backup, new_name, fs::copy_options::overwrite_existing);
                break;
            }
        }
    }

} // acmacs::file::backup

// ----------------------------------------------------------------------

void acmacs::file::write(std::string aFilename, std::string aData, ForceCompression aForceCompression, BackupFile aBackupFile)
{
    int f = -1;
    if (aFilename == "-") {
        f = 1;
    }
    else if (aFilename == "=") {
        f = 2;
    }
    else {
        if (aForceCompression == ForceCompression::Yes || (aFilename.size() > 3 && aFilename.substr(aFilename.size() - 3) == ".xz"))
            aData = xz_compress(aData);
        if (aBackupFile == BackupFile::Yes)
            backup(aFilename);
        f = open(aFilename.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
        if (f < 0)
            throw std::runtime_error(std::string("Cannot open ") + aFilename + ": " + strerror(errno));
    }
    if (::write(f, aData.c_str(), aData.size()) < 0)
        throw std::runtime_error(std::string("Cannot write ") + aFilename + ": " + strerror(errno));
    if (f > 2)
        close(f);

} // acmacs::file::write

// ----------------------------------------------------------------------

acmacs::file::temp::temp(std::string suffix)
    : name(make_template() + suffix), fd(mkstemps(const_cast<char*>(name.c_str()), static_cast<int>(suffix.size())))
{
    if (fd < 0)
        throw std::runtime_error(std::string("Cannot create temporary file using template ") + name + ": " + strerror(errno));

} // acmacs::file::temp::temp

// ----------------------------------------------------------------------

acmacs::file::temp::~temp()
{
    if (!name.empty())
        fs::remove(name.c_str());

} // acmacs::file::temp::~temp

// ----------------------------------------------------------------------

std::string acmacs::file::temp::make_template()
{
    const char* tdir = std::getenv("T");
    if (!tdir)
        tdir = std::getenv("TMPDIR");
    if (!tdir)
        tdir = "/tmp";
    return tdir + std::string{"/AD.XXXXXXXX"};

} // acmacs::file::temp::make_template

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
