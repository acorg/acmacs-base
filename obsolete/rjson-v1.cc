#error Obsolete

#include "read-file.hh"
#include "rjson-v2.hh"
#include "rjson-parser-pop-v1.hh"

// acmacs-chart-2
// hidb-5
// acmacs-draw
// acmacs-map-draw
// signature-page
// acmacs-webserver
// acmacs-api

// ----------------------------------------------------------------------

#ifdef __clang__
const char* std::bad_variant_access::what() const noexcept { return "bad_variant_access"; }
#endif

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

rjson::v1::value rjson::v1::sNull{rjson::v1::null{}};
rjson::v1::value rjson::v1::sEmptyArray{rjson::v1::array{}};
rjson::v1::value rjson::v1::sEmptyObject{rjson::v1::object{}};

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

rjson::v1::value rjson::v1::parse_string(std::string aJsonData, rjson::v1::remove_comments aRemoveComments)
{
    return parser_pop::parse_string(aJsonData, aRemoveComments);

} // rjson::v1::parse_string

// ----------------------------------------------------------------------

rjson::v1::value rjson::v1::parse_string(std::string_view aJsonData, rjson::v1::remove_comments aRemoveComments)
{
    return parser_pop::parse_string(aJsonData, aRemoveComments);

} // rjson::v1::parse_string

// ----------------------------------------------------------------------

rjson::v1::value rjson::v1::parse_string(const char* aJsonData, rjson::v1::remove_comments aRemoveComments)
{
    return parser_pop::parse_string(aJsonData, aRemoveComments);

} // rjson::v1::parse_string

// ----------------------------------------------------------------------

rjson::v1::value rjson::v1::parse_file(std::string aFilename, rjson::v1::remove_comments aRemoveComments)
{
    return parse_string(acmacs::file::read(aFilename), aRemoveComments);

} // rjson::v1::parse_file

// ----------------------------------------------------------------------

static inline bool is_simple(const rjson::v1::value& aValue, bool dive = true)
{
    auto visitor = [dive](auto&& arg) -> bool {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, rjson::v1::object>) {
            return (!arg.get_or_default(rjson::v1::object::force_pp_key, false)
                    && (arg.empty()
                        || (dive && std::all_of(arg.begin(), arg.end(), [](const auto& kv) -> bool { return is_simple(kv.second, false); }))));
        }
        else if constexpr (std::is_same_v<T, rjson::v1::array>) {
            return arg.empty() || (dive && std::all_of(arg.begin(), arg.end(), [](const auto& v) -> bool { return is_simple(v, false); }));
        }
        else {
            return true;
        }
    };
    return std::visit(visitor, aValue);
}

// ----------------------------------------------------------------------

std::string rjson::v1::object::to_json(bool space_after_comma) const
{
    std::string result(1, '{');
    size_t size_at_comma = 0;
    for (const auto& [key, val]: mContent) {
        if (key != force_pp_key && key != no_pp_key) {
            result.append(key.to_json());
            result.append(1, ':');
            if (space_after_comma)
                result.append(1, ' ');
            result.append(val.to_json());
            size_at_comma = result.size() + 1;
            result.append(1, ',');
            if (space_after_comma)
                result.append(1, ' ');
        }
    }
    if (result.back() == ',' || result.back() == ' ') {
        result.resize(size_at_comma);
        result.back() = '}';
    }
    else
        result.append(1, '}');
    return result;

} // rjson::v1::object::to_json

// ----------------------------------------------------------------------

std::string rjson::v1::object::to_json_pp(size_t indent, json_pp_emacs_indent emacs_indent, size_t prefix) const
{
    if (is_simple(*this) || get_or_default(rjson::v1::object::no_pp_key, false))
        return to_json(true);

    std::string result;
    if (emacs_indent == json_pp_emacs_indent::yes && indent) {
        result.append(1, '{');
        result.append(indent - 1, ' ');
        result.append("\"_\": \"-*- js-indent-level: " + std::to_string(indent) + " -*-\",\n");
    }
    else {
        result.append("{\n");
    }
    result.append(prefix + indent, ' ');
    size_t size_before_comma = result.size();
    for (const auto& [key, val]: mContent) {
        if (key != force_pp_key) {
            result.append(key.to_json());
            result.append(": ");
            result.append(val.to_json_pp(indent, json_pp_emacs_indent::no, prefix + indent));
            size_before_comma = result.size();
            result.append(",\n");
            result.append(prefix + indent, ' ');
        }
    }
    if (result.back() == ' ') {
        result.resize(size_before_comma);
        result.append(1, '\n');
        result.append(prefix, ' ');
        result.append(1, '}');
    }
    else
        result.append(1, '}');
    return result;

} // rjson::v1::object::to_json_pp

// ----------------------------------------------------------------------

void rjson::v1::array::remove_comments()
{
    for (auto& val: mContent)
        val.remove_comments();

} // rjson::v1::array::remove_comments

// ----------------------------------------------------------------------

std::string rjson::v1::array::to_json(bool space_after_comma) const
{
    std::string result(1, '[');
    for (const auto& val: mContent) {
        result.append(val.to_json());
        result.append(1, ',');
        if (space_after_comma)
            result.append(1, ' ');
    }
    if (space_after_comma && result.size() > 2 && result.back() == ' ') {
        result.resize(result.size() - 1);
        result.back() = ']';
    }
    else if (result.back() == ',')
        result.back() = ']';
    else
        result.append(1, ']');
    return result;

} // rjson::v1::array::to_json

// ----------------------------------------------------------------------

std::string rjson::v1::array::to_json_pp(size_t indent, json_pp_emacs_indent, size_t prefix) const
{
    if (is_simple(*this))
        return to_json(true);

    std::string result("[\n");
    result.append(prefix + indent, ' ');
    size_t size_before_comma = 0;
    for (const auto& val: mContent) {
        result.append(val.to_json_pp(indent, json_pp_emacs_indent::no, prefix + indent));
        size_before_comma = result.size();
        result.append(",\n");
        result.append(prefix + indent, ' ');
    }
    if (result.back() == ' ') {
        result.resize(size_before_comma);
        result.append(1, '\n');
        result.append(prefix, ' ');
        result.append(1, ']');
    }
    else
        result.append(1, ']');
    return result;

} // rjson::v1::array::to_json_pp

// ----------------------------------------------------------------------

rjson::v1::value& rjson::v1::value::update(const rjson::v1::value& to_merge)
{
    auto visitor = [this](auto&& arg1, auto&& arg2) {
        using T1 = std::decay_t<decltype(arg1)>;
        using T2 = std::decay_t<decltype(arg2)>;
        if constexpr (std::is_same_v<T1, T2>) {
            arg1.update(arg2);
        }
        else {
            throw merge_error(std::string{"cannot merge two rjson values of different types: %"} + this->to_json() + "% and %" + arg2.to_json());
        }
    };

    std::visit(visitor, *this, to_merge);
    return *this;

} // rjson::v1::value::update

// ----------------------------------------------------------------------

void rjson::v1::object::update(const rjson::v1::object& to_merge)
{
    for (const auto& [new_key, new_value]: to_merge) {
        try {
            rjson::v1::value& old_value = operator[](new_key);
            if (auto [old_n, old_i, new_n, new_i] = std::make_tuple(std::get_if<number>(&old_value), std::get_if<integer>(&old_value), std::get_if<number>(&new_value), std::get_if<integer>(&new_value));
                (old_n || old_i) && (new_n || new_i)) {
                  // number replaced with integer and vice versa
                set_field(new_key, new_value);
            }
            else {
                old_value.update(new_value);
            }
        }
        catch (field_not_found&) {
            set_field(new_key, new_value);
        }
    }

} // rjson::v1::object::update

// ----------------------------------------------------------------------

void rjson::v1::object::remove_comments()
{
    for (auto it = mContent.begin(); it != mContent.end(); /* no increment! */) {
        if (it->first.is_comment_key()) {
            it = mContent.erase(it);
        }
        else {
            it->second.remove_comments();
            ++it;
        }
    }

} // rjson::v1::object::remove_comments

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
