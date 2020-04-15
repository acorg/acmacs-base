#include "acmacs-base/messages.hh"
#include "acmacs-base/debug.hh"

// ----------------------------------------------------------------------

void acmacs::messages::v1::move(messages_t& target, messages_t&& from)
{
    std::move(std::begin(from), std::end(from), std::back_inserter(target));

} // acmacs::messages::v1::move

// ----------------------------------------------------------------------

void acmacs::messages::v1::move_and_add_source(messages_t& target, messages_t&& from, const position_t& source)
{
    for (auto& msg : from)
        target.emplace_back(std::move(msg)).source = source;

} // acmacs::messages::v1::move_and_add_source

// ----------------------------------------------------------------------

void acmacs::messages::v1::report_by_type(messages_t& messages)
{
    report(make_index(messages));

} // acmacs::messages::v1::report_by_type

// ----------------------------------------------------------------------

void acmacs::messages::v1::report(const index_t& index)
{
    if (!index.empty()) {
        AD_INFO("Total messages: {}  keys: {}", index.back().second - index.front().first, index.size());

        for (const auto& [first, last] : index) {
            AD_INFO("\"{}\" ({}):", first->key, last - first);
            for (auto cur = first; cur != last; ++cur) {
                fmt::print(stderr, "  {}", cur->value);
                if (!cur->source.filename.empty())
                    fmt::print(stderr, " @@ {}:{}", cur->source.filename, cur->source.line_no);
                if (!cur->code.filename.empty())
                    fmt::print(stderr, " @@ {}:{}", cur->code.filename, cur->code.line_no);
                fmt::print(stderr, "\n");
            }
            fmt::print(stderr, "\n");
        }
    }
    else
        AD_INFO("No messages");

} // acmacs::messages::v1::report

// ----------------------------------------------------------------------

acmacs::messages::v1::index_t acmacs::messages::v1::make_index(messages_t& messages)
{
    std::sort(std::begin(messages), std::end(messages), [](const auto& e1, const auto& e2) { return e1.key < e2.key; });
    std::vector<std::pair<iter_t, iter_t>> index;
    std::string_view prev_key;
    for (iter_t cur = messages.cbegin(); cur != messages.cend(); ++cur) {
        if (prev_key != cur->key) {
            prev_key = cur->key;
            if (!index.empty())
                index.back().second = cur;
            index.emplace_back(cur, cur);
        }
    }
    if (!index.empty())
        index.back().second = messages.end();
    return index;

} // acmacs::messages::v1::make_index

// ----------------------------------------------------------------------

acmacs::messages::v1::index_entry_t acmacs::messages::v1::find(key_t key, const index_t& index)
{
    if (const auto found = std::find_if(std::begin(index), std::end(index), [key](const auto& entry) { return entry.first->key == key; }); found != std::end(index))
        return *found;
    else
        return {index.back().second, index.back().second};

} // acmacs::messages::v1::find

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
