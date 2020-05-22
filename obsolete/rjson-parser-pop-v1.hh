#pragma once
#error Obsolete

// ----------------------------------------------------------------------

namespace rjson
{
    namespace v1
    {
        namespace parser_pop
        {
            value parse_string(std::string aJsonData, remove_comments aRemoveComments);
            value parse_string(const std::string_view& aJsonData, remove_comments aRemoveComments);
            value parse_string(const char* aJsonData, remove_comments aRemoveComments);

        } // namespace parser_pop
    }     // namespace v1
} // namespace rjson

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
