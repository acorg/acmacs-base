#pragma once

#include "acmacs-base/string.hh"
#include "acmacs-base/sfinae.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    class CsvWriter
    {
      public:
        operator std::string() const { return mData; }
        operator std::string_view() const { return mData; }

        CsvWriter& new_row()
        {
            mData.append(1, '\n');
            return *this;
        }

        void add_field(std::string aField)
        {
            // https://www.ietf.org/rfc/rfc4180.txt
            if (aField.find(',') != std::string::npos || aField.find('\n') != std::string::npos || aField.find('"') != std::string::npos) {
                aField = '"' + ::string::replace(aField, "\"", "\"\"") + '"';
            }
            if (!mData.empty() && mData.back() != '\n')
                mData.append(1, ',');
            mData.append(aField);
        }

        void add_empty_field()
        {
            if (!mData.empty() && mData.back() != '\n')
                mData.append(1, ',');
        }

        CsvWriter& field()
        {
            add_empty_field();
            return *this;
        }

        // template <typename S, typename=sfinae::string_only_t<S>> CsvWriter& field(S field)
        template <typename S> CsvWriter& field(S field)
        {
            add_field(to_string(field));
            return *this;
        }

        // template <typename S, typename=sfinae::string_only_t<S>> CsvWriter& operator<<(S data) { return field(data); }
        template <typename S> CsvWriter& operator<<(S data) { return field(data); }

        enum empty_field_ { empty_field };
        CsvWriter& operator<<(empty_field_) { return field(); }

        enum end_of_row_ { end_of_row };
        CsvWriter& operator<<(end_of_row_) { return new_row(); }

      private:
        std::string mData;

    }; // class CsvWriter

    inline std::ostream& operator<<(std::ostream& out, const CsvWriter& csv) { return out << static_cast<std::string_view>(csv); }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
