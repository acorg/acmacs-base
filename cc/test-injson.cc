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

    template <typename Iter> void json_string(Iter first, Iter last)
    {
        ++strings;
    }

    template <typename Iter> void json_integer(Iter first, Iter last)
    {
        std::strtol(&*first, nullptr, 0);
        ++integers;
    }

    template <typename Iter> void json_real(Iter first, Iter last)
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

int main(int argc, const char* const * argv)
{
    if (argc != 2) {
        fmt::print(stderr, "Usage: {} <file.json[.xz]>\n", argv[0]);
        return 1;
    }
    try {
        const std::string data = acmacs::file::read(argv[1]);
        EmptySink sink;
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
