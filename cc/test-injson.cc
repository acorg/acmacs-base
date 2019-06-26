#include <stack>
#include <vector>

#include "acmacs-base/in-json.hh"
#include "acmacs-base/read-file.hh"

// ----------------------------------------------------------------------

class EmptySink
{
  public:
    void injson_object_start()
    {
        ++objects;
    }

    void injson_object_end()
    {
        ++object_balance;
    }

    void injson_array_start()
    {
        ++arrays;
    }

    void injson_array_end()
    {
        ++array_balance;
    }

    template <typename Iter> void injson_string(Iter /*first*/, Iter /*last*/)
    {
        ++strings;
    }

    template <typename Iter> void injson_integer(Iter first, Iter /*last*/)
    {
        std::strtol(&*first, nullptr, 0);
        ++integers;
    }

    template <typename Iter> void injson_real(Iter first, Iter /*last*/)
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
    virtual const char* injson_name() = 0;
    virtual SeqdbStackEntry& injson_put_object() = 0;
    virtual void injson_put_string(std::string_view data) { key_ = data; }
    virtual void injson_put_integer(long /*data*/) { throw seqdb_paring_error("SeqdbStackEntry::injson_put_integer"); }
    // virtual void injson_put_real(double /*data*/) { throw seqdb_paring_error("SeqdbStackEntry::injson_put_real"); }
    virtual void injson_put_array() { throw seqdb_paring_error("SeqdbStackEntry::injson_put_array"); }
    virtual void injson_pop_array() { throw seqdb_paring_error("SeqdbStackEntry::injson_pop_array"); }

  protected:
    std::string_view key_{};
};

class SeqdbSeqLabs : public SeqdbStackEntry
{
  public:
    SeqdbSeqLabs(std::vector<std::pair<std::string, std::vector<std::string>>>& target) : target_{target} {}

    const char* injson_name() override { return "SeqdbSeqLabs"; }
    SeqdbStackEntry& injson_put_object() override { throw seqdb_paring_error("SeqdbSeqLabs: unexpected subobject"); }

    void injson_put_array() override
    {
    }

    void injson_pop_array() override
    {
    }

    void injson_put_string(std::string_view data) override
    {
        if (key_.empty()) {
            SeqdbStackEntry::injson_put_string(data);
            target_.emplace_back(std::string(data), std::vector<std::string>{});
        }
        else
            target_.back().second.emplace_back(data);
    }

  private:
    std::vector<std::pair<std::string, std::vector<std::string>>>& target_;
};

class SeqdbSeq : public SeqdbStackEntry
{
  public:
    const char* injson_name() override { return "SeqdbSeq"; }

    SeqdbStackEntry& injson_put_object() override
    {
        // if (key_[0] == 'l') {
            key_ = std::string_view{};
            return lab_id_entry_;
        // }
        // else
        //     throw seqdb_paring_error("SeqdbEntry: unexpected sub-object, key: " + std::string(key_));
    }

    void injson_put_array() override
    {
        // if (key_.size() != 1 || (key_[0] != 'p' && key_[0] != 'c' && key_[0] != 'h' && key_[0] != 'r'))
        //     throw seqdb_paring_error("SeqdbSeq: unexpected array, key: " + std::string(key_));
    }

    void injson_pop_array() override
    {
        // if (key_.size() != 1 || (key_[0] != 'p' && key_[0] != 'c' && key_[0] != 'h' && key_[0] != 'r'))
        //     throw seqdb_paring_error("SeqdbSeq: unexpected array, key: " + std::string(key_));
        key_ = std::string_view{};
    }

    void injson_put_string(std::string_view data) override
    {
        if (key_.empty())
            SeqdbStackEntry::injson_put_string(data);
        else
        // else if (key_.size() == 1) {
            switch (key_[0]) {
                case 'p':
                    passages_.emplace_back(data);
                    break;
                case 'r':
                    reassortants_.emplace_back(data);
                    break;
                case 'c':
                    clades_.emplace_back(data);
                    break;
                case 'h':
                    hi_names_.emplace_back(data);
                    break;
                case 'g':
                    gene_ = data;
                    key_ = std::string_view{};
                    break;
                case 'a':
                    amino_acids_ = data;
                    key_ = std::string_view{};
                    break;
                case 'n':
                    nucs_ = data;
                    key_ = std::string_view{};
                    break;
                default:
                    throw seqdb_paring_error("SeqdbSeq: unexpected key: " + std::string(data));
            }
        // }
        // else
        //     throw seqdb_paring_error("SeqdbSeq: unexpected key: [" + std::string(data) + ']');
    }

    void injson_put_integer(long data) override
    {
        // if (key_.size() == 1) {
            switch (key_[0]) {
                case 's':
                    a_shift_ = data;
                    key_ = std::string_view{};
                    break;
                case 't':
                    n_shift_ = data;
                    key_ = std::string_view{};
                    break;
              default:
                  throw seqdb_paring_error("SeqdbSeq: unexpected integer, key: " + std::string(key_));
            }
        // }
        // else
        //     throw seqdb_paring_error("SeqdbSeq: unexpected integer, key: " + std::string(key_));
    }
  private:
    std::string amino_acids_;
    long a_shift_{0};
    std::string nucs_;
    long n_shift_{0};
    std::vector<std::string> passages_;
    std::vector<std::string> reassortants_;
    std::vector<std::string> clades_;
    std::string gene_;
    std::vector<std::string> hi_names_;
    std::vector<std::pair<std::string, std::vector<std::string>>> lab_ids_;
    SeqdbSeqLabs lab_id_entry_{lab_ids_};
};

class SeqdbEntry : public SeqdbStackEntry
{
  public:
    const char* injson_name() override { return "SeqdbEntry"; }

    SeqdbStackEntry& injson_put_object() override
    {
        // if (key_[0] == 's')
            return seqs_.emplace_back();
        // else
        //     throw seqdb_paring_error("SeqdbEntry: unexpected sub-object, key: " + std::string(key_));
    }

    void injson_put_array() override
    {
        // if (key_.size() != 1 || (key_[0] != 'd' && key_[0] != 's'))
        //     throw seqdb_paring_error("SeqdbEntry: unexpected array, key: " + std::string(key_));
    }

    void injson_pop_array() override
    {
        // if (key_.size() != 1 || (key_[0] != 'd' && key_[0] != 's'))
        //     throw seqdb_paring_error("SeqdbEntry: unexpected array, key: " + std::string(key_));
        key_ = std::string_view{};
    }

    void injson_put_string(std::string_view data) override
    {
        if (key_.empty())
            SeqdbStackEntry::injson_put_string(data);
        else
            // if (key_.size() == 1) {
            switch (key_[0]) {
                case 'N':
                    name_ = data;
                    key_ = std::string_view{};
                    // fmt::print(stderr, "NAME: {}\n", name_);
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
        // }
        // else
        //     throw seqdb_paring_error("SeqdbEntry: unexpected key: " + std::string(data));
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
    const char* injson_name() override { return "Seqdb"; }

    SeqdbStackEntry& injson_put_object() override
    {
        return entries_.emplace_back();
    }

    void injson_put_string(std::string_view data) override
    {
        if (key_ == "  version") {
            if (data != "sequence-database-v2")
                throw seqdb_paring_error("unsupported version: " + std::string(data));
            key_ = std::string_view{};
        }
        else if (key_.empty())
            SeqdbStackEntry::injson_put_string(data);
        else if (key_ == "  date" || key_ == "_")
            key_ = std::string_view{};
        else
            throw seqdb_paring_error("unsupported field: " + std::string(data));
    }

    void injson_put_array() override
    {
        // if (key_ != "data")
        //     throw seqdb_paring_error("Seqdb: unexpected array, key: " + std::string(key_));
    }

    void injson_pop_array() override
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
    void injson_object_start()
    {
        if (target_.empty())
            target_.push(seqdb_);
        else
            target_.push(target_.top().get().injson_put_object());
    }

    void injson_object_end()
    {
        target_.pop();
        // fmt::print(stderr, "injson_object_end --> {}\n", target_.top().get().injson_name());
    }

    void injson_array_start()
    {
        target_.top().get().injson_put_array();
    }

    void injson_array_end()
    {
        target_.top().get().injson_pop_array();

    }

    template <typename Iter> void injson_string(Iter first, Iter last)
    {
        target_.top().get().injson_put_string({&*first, static_cast<size_t>(last - first)});
    }

    template <typename Iter> void injson_integer(Iter first, Iter /*last*/)
    {
        target_.top().get().injson_put_integer(std::strtol(&*first, nullptr, 0));
    }

    template <typename Iter> void injson_real(Iter /*first*/, Iter /*last*/)
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
        in_json::parse(sink, std::begin(data), std::end(data));
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
