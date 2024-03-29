// #include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/mman.h>

#include "acmacs-base/filesystem.hh"
#include "acmacs-base/string-compare.hh"
#include "acmacs-base/fmt.hh"
#include "acmacs-base/xz.hh"
#include "acmacs-base/bzip2.hh"
#include "acmacs-base/gzip.hh"
#include "acmacs-base/brotli.hh"
#include "acmacs-base/date.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/temp-file.hh"

// ----------------------------------------------------------------------

acmacs::file::read_access::read_access(std::string_view aFilename)
{
    if (aFilename == "-") {
        constexpr size_t chunk_size = 1024 * 100;
        data_.resize(chunk_size);
        ssize_t start = 0;
        for (;;) {
            if (const auto bytes_read = ::read(0, data_.data() + start, chunk_size); bytes_read > 0) {
                start += bytes_read;
                data_.resize(static_cast<size_t>(start));
            }
            else
                break;
        }
    }
    else if (fs::exists(aFilename)) {
        len_ = fs::file_size(aFilename);
        fd = ::open(aFilename.data(), O_RDONLY);
        if (fd >= 0) {
            mapped_ = reinterpret_cast<char*>(mmap(nullptr, len_, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0));
            if (mapped_ == MAP_FAILED)
                throw cannot_read{fmt::format("{}: {}", aFilename, strerror(errno))};
        }
        else {
            throw not_opened{fmt::format("{}: {}", aFilename, strerror(errno))};
        }
    }
    else {
        throw not_found{std::string{aFilename}};
    }

} // acmacs::file::read_access

// ----------------------------------------------------------------------

acmacs::file::read_access::read_access(read_access&& other)
    : fd{other.fd}, len_{other.len_}, mapped_{other.mapped_}, data_{other.data_}
{
    other.fd = -1;
    other.len_ = 0;
    other.mapped_ = nullptr;
    other.data_.clear();

} // acmacs::file::read_access::read_access

// ----------------------------------------------------------------------

acmacs::file::read_access::~read_access()
{
    if (fd > 2) {
        if (mapped_)
            munmap(mapped_, len_);
        close(fd);
    }

} // acmacs::file::read_access::~read_access

// ----------------------------------------------------------------------

acmacs::file::read_access& acmacs::file::read_access::operator=(read_access&& other)
{
    fd = other.fd;
    len_ = other.len_;
    mapped_ = other.mapped_;
    data_ = other.data_;

    other.fd = -1;
    other.len_ = 0;
    other.mapped_ = nullptr;
    other.data_.clear();

    return *this;

} // acmacs::file::read_access::operator=

// ----------------------------------------------------------------------

std::string acmacs::file::decompress_if_necessary(std::string_view aSource)
{
    if (xz_compressed(aSource.data()))
        return xz_decompress(aSource);
    else if (brotli_compressed(aSource))
        return brotli_decompress(aSource);
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

void acmacs::file::backup(std::string_view _to_backup, std::string_view _backup_dir, backup_move bm)
{
    const fs::path to_backup{_to_backup}, backup_dir{_backup_dir};

    if (fs::exists(to_backup)) {
        try {
            fs::create_directory(backup_dir);
        }
        catch (std::exception& err) {
            fmt::print(stderr, "> ERROR cannot create directory {}: {}\n", backup_dir.native(), err);
            throw;
        }

        auto extension = to_backup.extension();
        auto stem = to_backup.stem();
        if ((extension == ".bz2" || extension == ".xz" || extension == ".gz") && !stem.extension().empty()) {
            extension = stem.extension();
            extension += to_backup.extension();
            stem = stem.stem();
        }
        const auto today = date::display(date::today(), "%Y%m%d");
        for (int version = 1; version < 1000; ++version) {
            char infix[4];
            std::snprintf(infix, 4, "%03d", version);
            fs::path new_name = backup_dir / (stem.string() + ".~" + today + '-' + infix + '~' + extension.string());
            if (!fs::exists(new_name) || version == 999) {
                try {
                    if (bm == backup_move::yes)
                        fs::rename(to_backup, new_name); // if new_name exists it will be removed before doing rename
                    else
                        fs::copy_file(to_backup, new_name, fs::copy_options::overwrite_existing);
                }
                catch (std::exception& err) {
                    fmt::print(stderr, ">> WARNING backing up \"{}\" to \"{}\" failed: {}\n", to_backup.native(), new_name.native(), err);
                }
                break;
            }
        }
    }

} // acmacs::file::backup

// ----------------------------------------------------------------------

void acmacs::file::backup(const std::string_view to_backup, backup_move bm)
{
    backup(to_backup, (fs::path{to_backup}.parent_path() / ".backup").native(), bm);

} // acmacs::file::backup

// ----------------------------------------------------------------------

void acmacs::file::write(std::string_view aFilename, std::string_view aData, force_compression aForceCompression, backup_file aBackupFile)
{
    using namespace std::string_view_literals;
    int f = -1;
    if (aFilename == "-") {
        f = 1;
    }
    else if (aFilename == "=") {
        f = 2;
    }
    else if (aFilename == "/") {
        f = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT, 0644);
        if (f < 0)
            throw std::runtime_error(fmt::format("Cannot open /dev/null: {}", strerror(errno)));
    }
    else {
        if (aBackupFile == backup_file::yes && aFilename.substr(0, 4) != "/dev") // allow writing to /dev/ without making backup attempt
            backup(aFilename);
        f = open(aFilename.data(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
        if (f < 0)
            throw std::runtime_error(fmt::format("Cannot open {}: {}", aFilename, strerror(errno)));
    }
    try {
        if (aForceCompression == force_compression::yes || (aFilename.size() > 3 && (acmacs::string::endswith(aFilename, ".xz"sv) || acmacs::string::endswith(aFilename, ".gz"sv)))) {
            const auto compressed = acmacs::string::endswith(aFilename, ".gz"sv) ? gzip_compress(aData) : xz_compress(aData);
            if (::write(f, compressed.data(), compressed.size()) < 0)
                throw std::runtime_error(fmt::format("Cannot write {}: {}", aFilename, strerror(errno)));
        }
        else {
            if (::write(f, aData.data(), aData.size()) < 0)
                throw std::runtime_error(fmt::format("Cannot write {}: {}", aFilename, strerror(errno)));
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

acmacs::file::temp::temp(std::string prefix, std::string suffix, bool autoremove)
    : name(make_template(prefix) + suffix), autoremove_{autoremove}, fd(mkstemps(const_cast<char*>(name.c_str()), static_cast<int>(suffix.size())))
{
    if (fd < 0)
        throw std::runtime_error(std::string("Cannot create temporary file using template ") + name + ": " + strerror(errno));

} // acmacs::file::temp::temp

// ----------------------------------------------------------------------

acmacs::file::temp::temp(std::string suffix, bool autoremove)
    : acmacs::file::temp::temp("AD.", suffix, autoremove)
{

} // acmacs::file::temp::temp

// ----------------------------------------------------------------------

acmacs::file::temp::~temp()
{
    if (autoremove_ && !name.empty())
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
