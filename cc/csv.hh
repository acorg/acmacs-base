#pragma once

#include "acmacs-base/string.hh"
#include "acmacs-base/log.hh"

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
            if (last_field_was_empty_)
                mData.append(1, ',');
            last_field_was_empty_ = false;
            mData.append(1, '\n');
            return *this;
        }

        void add_field(std::string_view aField)
        {
            if (last_field_was_empty_ || (!mData.empty() && mData.back() != '\n' && mData.back() != ','))
                mData.append(1, ',');
            if (aField.find(',') != std::string_view::npos || aField.find('\n') != std::string_view::npos || aField.find('"') != std::string_view::npos) {
                // https://www.ietf.org/rfc/rfc4180.txt
                mData.append(fmt::format("\"{}\"", ::string::replace(aField, "\"", "\"\"")));
            }
            else
                mData.append(aField);
            last_field_was_empty_ = aField.empty();
        }

        void add_empty_field()
        {
            if (last_field_was_empty_)
                mData.append(1, ',');
            last_field_was_empty_ = true;
        }

        CsvWriter& field()
        {
            add_empty_field();
            return *this;
        }

        // template <typename S, typename=sfinae::string_only_t<S>> CsvWriter& field(S field)
        template <typename S> CsvWriter& field(S field)
        {
            add_field(fmt::format("{}", field));
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
        bool last_field_was_empty_{false};

    }; // class CsvWriter

    inline std::ostream& operator<<(std::ostream& out, const CsvWriter& csv) { return out << static_cast<std::string_view>(csv); }

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
