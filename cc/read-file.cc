#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/mman.h>

#include "acmacs-base/xz.hh"
#include "acmacs-base/bzip2.hh"
#include "acmacs-base/gzip.hh"
#include "acmacs-base/date.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/read-file.hh"

// ----------------------------------------------------------------------

acmacs::file::read_access::read_access(std::string_view aFilename)
{
    if (fs::exists(aFilename)) {
        len = fs::file_size(aFilename);
        fd = ::open(aFilename.data(), O_RDONLY);
        if (fd >= 0) {
            mapped = reinterpret_cast<char*>(mmap(nullptr, len, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0));
            if (mapped == MAP_FAILED)
                throw cannot_read(::string::concat(aFilename, ": ", strerror(errno)));
        }
        else {
            throw not_opened(::string::concat(aFilename, ": ", strerror(errno)));
        }
    }
    else {
        throw not_found{std::string{aFilename}};
    }

} // acmacs::file::read_access

// ----------------------------------------------------------------------

acmacs::file::read_access::read_access(read_access&& other)
    : fd{other.fd}, len{other.len}, mapped{other.mapped}
{
    other.fd = -1;
    other.len = 0;
    other.mapped = nullptr;

} // acmacs::file::read_access::read_access

// ----------------------------------------------------------------------

acmacs::file::read_access::~read_access()
{
    if (fd >= 0) {
        if (mapped)
            munmap(mapped, len);
        close(fd);
    }

} // acmacs::file::read_access::~read_access

// ----------------------------------------------------------------------

acmacs::file::read_access& acmacs::file::read_access::operator=(read_access&& other)
{
    fd = other.fd;
    len = other.len;
    mapped = other.mapped;

    other.fd = -1;
    other.len = 0;
    other.mapped = nullptr;

    return *this;

} // acmacs::file::read_access::operator=

// ----------------------------------------------------------------------

std::string acmacs::file::decompress_if_necessary(std::string_view aSource)
{
    if (xz_compressed(aSource.data()))
        return xz_decompress(aSource);
    else if (bz2_compressed(aSource.data()))
        return bz2_decompress(aSource);
    else if (gzip_compressed(aSource.data())) {
        return gzip_decompress(aSource);
    }
    else
        return std::string(aSource);

} // acmacs::file::decompress_if_necessary

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
    return decompress_if_necessary(std::string_view(buffer));

} // acmacs::file::read_from_file_descriptor

// ----------------------------------------------------------------------

void acmacs::file::backup(const fs::path& to_backup, const fs::path& backup_dir, backup_move bm)
{
    if (fs::exists(to_backup)) {
        try {
            fs::create_directory(backup_dir);
        }
        catch (std::exception& err) {
            std::cerr << "ERROR: cannot create directory " << backup_dir << ": " << err.what() << '\n';
            throw;
        }

        auto extension = to_backup.extension();
        auto stem = to_backup.stem();
        if ((extension == ".bz2" || extension == ".xz" || extension == ".gz") && !stem.extension().empty()) {
            extension = stem.extension();
            extension += to_backup.extension();
            stem = stem.stem();
        }
        const auto today = Date::today().display("%Y%m%d");
        for (int version = 1; version < 1000; ++version) {
            char infix[4];
            std::sprintf(infix, "%03d", version);
            fs::path new_name = backup_dir / (stem.string() + ".~" + today + '-' + infix + '~' + extension.string());
            if (!fs::exists(new_name) || version == 999) {
                try {
                    if (bm == backup_move::yes)
                        fs::rename(to_backup, new_name); // if new_name exists it will be removed before doing rename
                    else
                        fs::copy_file(to_backup, new_name, fs::copy_options::overwrite_existing);
                }
                catch (std::exception& err) {
                    std::cerr << "WARNING: backing up \"" << to_backup << "\" to \"" << new_name << "\" failed: " << err.what() << '\n';
                }
                break;
            }
        }
    }

} // acmacs::file::backup

// ----------------------------------------------------------------------

void acmacs::file::write(std::string aFilename, std::string_view aData, force_compression aForceCompression, backup_file aBackupFile)
{
    int f = -1;
    if (aFilename == "-") {
        f = 1;
    }
    else if (aFilename == "=") {
        f = 2;
    }
    else {
        if (aBackupFile == backup_file::yes && aFilename.substr(0, 4) != "/dev") // allow writing to /dev/ without making backup attempt
            backup(aFilename);
        f = open(aFilename.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
        if (f < 0)
            throw std::runtime_error(std::string("Cannot open ") + aFilename + ": " + strerror(errno));
    }
    try {
        if (aForceCompression == force_compression::yes || (aFilename.size() > 3 && (string::ends_with(aFilename, ".xz") || string::ends_with(aFilename, ".gz")))) {
            const auto compressed = string::ends_with(aFilename, ".gz") ? gzip_compress(aData) : xz_compress(aData);
            if (::write(f, compressed.data(), compressed.size()) < 0)
                throw std::runtime_error(std::string("Cannot write ") + aFilename + ": " + strerror(errno));
        }
        else {
            if (::write(f, aData.data(), aData.size()) < 0)
                throw std::runtime_error(std::string("Cannot write ") + aFilename + ": " + strerror(errno));
        }
        if (f > 2)
            close(f);
    }
    catch (std::exception&) {
        if (f > 2)
            close(f);
        throw;
    }

} // acmacs::file::write

// ----------------------------------------------------------------------

acmacs::file::temp::temp(std::string prefix, std::string suffix)
    : name(make_template(prefix) + suffix), fd(mkstemps(const_cast<char*>(name.c_str()), static_cast<int>(suffix.size())))
{
    if (fd < 0)
        throw std::runtime_error(std::string("Cannot create temporary file using template ") + name + ": " + strerror(errno));

} // acmacs::file::temp::temp

// ----------------------------------------------------------------------

acmacs::file::temp::temp(std::string suffix)
    : acmacs::file::temp::temp("AD.", suffix)
{

} // acmacs::file::temp::temp

// ----------------------------------------------------------------------

acmacs::file::temp::~temp()
{
    if (!name.empty())
        fs::remove(name.c_str());

} // acmacs::file::temp::~temp

// ----------------------------------------------------------------------

std::string acmacs::file::temp::make_template(std::string prefix)
{
    const char* tdir = std::getenv("T");
    if (!tdir || *tdir != '/')
        tdir = std::getenv("TMPDIR");
    if (!tdir)
        tdir = "/tmp";
    return tdir + std::string{"/"} + prefix + "_.XXXXXXXX";

} // acmacs::file::temp::make_template

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
