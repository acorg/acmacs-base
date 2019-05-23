#include <stack>
#include <vector>

#include "acmacs-base/injson.hh"
#include "acmacs-base/read-file.hh"

// ----------------------------------------------------------------------

class EmptySink
{
  public:
    void json_object_start()
    {
        ++objects;
    }

    void json_object_end()
    {
        ++object_balance;
    }

    void json_array_start()
    {
        ++arrays;
    }

    void json_array_end()
    {
        ++array_balance;
    }

    template <typename Iter> void json_string(Iter /*first*/, Iter /*last*/)
    {
        ++strings;
    }

    template <typename Iter> void json_integer(Iter first, Iter /*last*/)
    {
        std::strtol(&*first, nullptr, 0);
        ++integers;
    }

    template <typename Iter> void json_real(Iter first, Iter /*last*/)
    {
        std::strtod(&*first, nullptr);
        ++reals;
    }

    void report() const
    {
        fmt::print("EmptySink: strings:{} ints:{} reals:{} objects:{} (balance:{}) arrays:{} (balance: {})\n", strings, integers, reals, objects, objects - object_balance, arrays, arrays - array_balance);
    }

  private:
    int strings = 0;
    int objects = 0;
    int arrays = 0;
    int integers = 0;
    int reals = 0;
    int object_balance = 0;
    int array_balance = 0;
};

// ----------------------------------------------------------------------

class seqdb_paring_error : public std::runtime_error
{
  public:
    template <typename S> seqdb_paring_error(S&& message) : std::runtime_error(fmt::format("seqdb_paring_error: {}", message)) {}
};

class SeqdbStackEntry
{
  public:
    SeqdbStackEntry() = default;
    SeqdbStackEntry(const SeqdbStackEntry&) = default;
    virtual ~SeqdbStackEntry() = default;
    virtual SeqdbStackEntry& subobject() = 0;
    virtual void put_string(std::string_view data) { key_ = data; }
    virtual void put_integer(long /*data*/) { throw seqdb_paring_error("SeqdbStackEntry::put_integer"); }
    // virtual void put_real(double /*data*/) { throw seqdb_paring_error("SeqdbStackEntry::put_real"); }
    virtual void put_array() { throw seqdb_paring_error("SeqdbStackEntry::put_array"); }
    virtual void pop_array() { throw seqdb_paring_error("SeqdbStackEntry::pop_array"); }

  protected:
    std::string_view key_{};
};

class SeqdbSeq : public SeqdbStackEntry
{
  public:
    SeqdbStackEntry& subobject() override { return *this; }
};

class SeqdbEntry : public SeqdbStackEntry
{
  public:
    SeqdbStackEntry& subobject() override { return *this; }

    void put_array() override
    {
        if (key_.size() != 1 && key_[0] != 'd')
            throw seqdb_paring_error("SeqdbEntry: unexpected array, key: " + std::string(key_));
    }

    void pop_array() override
    {
        if (key_.size() != 1 && key_[0] != 'd')
            throw seqdb_paring_error("SeqdbEntry: unexpected array, key: " + std::string(key_));
        key_ = std::string_view{};
    }

    void put_string(std::string_view data) override
    {
        if (key_.empty())
            SeqdbStackEntry::put_string(data);
        else if (key_.size() == 1) {
            switch (key_[0]) {
                case 'N':
                    name_ = data;
                    key_ = std::string_view{};
                    break;
                case 'C':
                    continent_ = data;
                    key_ = std::string_view{};
                    break;
                case 'c':
                    country_ = data;
                    key_ = std::string_view{};
                    break;
                case 'd':
                    dates_.emplace_back(data);
                    break;
                case 'l':
                    lineage_ = data;
                    key_ = std::string_view{};
                    break;
                case 'v':
                    virus_type_ = data;
                    key_ = std::string_view{};
                    break;
                default:
                    throw seqdb_paring_error("SeqdbEntry: unexpected key: " + std::string(data));
            }
        }
        else
            throw seqdb_paring_error("SeqdbEntry: unexpected key: " + std::string(data));
    }

  private:
    std::string name_;
    std::string continent_;
    std::string country_;
    std::vector<std::string> dates_;
    std::string lineage_;
    std::string virus_type_;
    std::vector<SeqdbSeq> seqs_;
};

class Seqdb : public SeqdbStackEntry
{
  public:
    SeqdbStackEntry& subobject() override
    {
        return entries_.emplace_back();
    }

    void put_string(std::string_view data) override
    {
        if (key_ == "  version") {
            if (data != "sequence-database-v2")
                throw seqdb_paring_error("unsupported version: " + std::string(data));
            key_ = std::string_view{};
        }
        else if (key_.empty())
            SeqdbStackEntry::put_string(data);
        else if (key_ == "  date" || key_ == "_")
            key_ = std::string_view{};
        else
            throw seqdb_paring_error("unsupported field: " + std::string(data));
    }

    void put_array() override
    {
        if (key_ != "data")
            throw seqdb_paring_error("Seqdb: unexpected array, key: " + std::string(key_));
    }

    void pop_array() override
    {
        key_ = std::string_view{};
    }

    const auto& entries() const { return entries_; }

  private:
    std::vector<SeqdbEntry> entries_;
};

class SeqdbSink
{
  public:
    void json_object_start()
    {
        if (target_.empty())
            target_.push(seqdb_);
        else
            target_.push(target_.top().get().subobject());
    }

    void json_object_end()
    {
        target_.pop();
    }

    void json_array_start()
    {
        target_.top().get().put_array();
    }

    void json_array_end()
    {
        target_.top().get().pop_array();

    }

    template <typename Iter> void json_string(Iter first, Iter last)
    {
        target_.top().get().put_string({&*first, static_cast<size_t>(last - first)});
    }

    template <typename Iter> void json_integer(Iter first, Iter /*last*/)
    {
        target_.top().get().put_integer(std::strtol(&*first, nullptr, 0));
    }

    template <typename Iter> void json_real(Iter /*first*/, Iter /*last*/)
    {
    }

    void report() const
    {
        fmt::print("SeqdbSink: entries:{}\n", seqdb_.entries().size());
    }

  private:
    Seqdb seqdb_;
    std::stack<std::reference_wrapper<SeqdbStackEntry>> target_;
};

// ----------------------------------------------------------------------

int main(int argc, const char* const * argv)
{
    if (argc != 2) {
        fmt::print(stderr, "Usage: {} <file.json[.xz]>\n", argv[0]);
        return 1;
    }
    try {
        const std::string data = acmacs::file::read(argv[1]);
        // EmptySink sink;
        SeqdbSink sink;
        injson::parse(sink, std::begin(data), std::end(data));
        sink.report();
        return 0;
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err.what());
        return 2;
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
