#pragma once

#include "acmacs-base/string.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    class CsvWriter
    {
     public:
        inline operator std::string() const { return mData; }
        inline operator std::string_view() const { return mData; }

        inline void new_row() { mData.append(1, '\n'); }

        inline void add_field(std::string aField)
            {
                  // https://www.ietf.org/rfc/rfc4180.txt
                if (aField.find(',') != std::string::npos || aField.find('\n') != std::string::npos || aField.find('"') != std::string::npos) {
                    aField = '"' + string::replace(aField, "\"", "\"\"") + '"';
                }
                if (!mData.empty() && mData.back() != '\n')
                    mData.append(1, ',');
                mData.append(aField);
            }

        inline void add_empty_field() { if (!mData.empty() && mData.back() != '\n') mData.append(1, ','); }

     private:
        std::string mData;

    }; // class CsvWriter

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
