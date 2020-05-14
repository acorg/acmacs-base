#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <iterator>
#include <stdexcept>
#include <algorithm>
#include <cctype>

#include "acmacs-base/fmt.hh"
#include "acmacs-base/string-from-chars.hh"
#include "acmacs-base/string-strip.hh"

// ----------------------------------------------------------------------

namespace acmacs::string
{
    enum class Split { RemoveEmpty, KeepEmpty, StripRemoveEmpty, StripKeepEmpty };

    class split_error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    namespace internal
    {
        template <typename S> class split_iterator
        {
         public:
            using iterator_category = std::input_iterator_tag;
            using value_type = std::string_view;
            using difference_type = ssize_t;
            using pointer = std::string_view*;
            using reference = std::string_view&;

            inline split_iterator() : mInputEnd(nullptr), mKeepEmpty(Split::RemoveEmpty), mBegin(nullptr), mEnd(nullptr) {}
            inline split_iterator(const S& s, std::string_view delim, Split keep_empty)
                : mInputEnd(&*s.cend()), mDelim(delim), mKeepEmpty(keep_empty), mBegin(s.data()), mEnd(nullptr)
                {
                    if (mDelim.empty()) {
                        mEnd = mInputEnd;
                    }
                    else
                        next();
                }

            inline value_type operator*() noexcept
                {
                    value_type res{mBegin, static_cast<typename value_type::size_type>(mEnd - mBegin)};
                    if (mKeepEmpty == Split::StripKeepEmpty || mKeepEmpty == Split::StripRemoveEmpty)
                        res = acmacs::string::strip(res);
                    return res;
                }

            inline split_iterator& operator++() noexcept
                {
                    if (mEnd != nullptr) {
                        if (mDelim.empty())
                            set_end();
                        else
                            next();
                    }
                    return *this;
                }

            split_iterator operator++(int) noexcept = delete;

            inline bool operator==(const split_iterator& other) const noexcept
                {
                    return mEnd == other.mEnd && (mEnd == nullptr || (mInputEnd == other.mInputEnd && mDelim == other.mDelim && mKeepEmpty == other.mKeepEmpty && mBegin == other.mBegin));
                }

            inline bool operator!=(const split_iterator& other) const noexcept { return !operator==(other); }

         private:
            const char* mInputEnd;
            const std::string_view mDelim;
            const Split mKeepEmpty;
            const char* mBegin;
            const char* mEnd;

            inline void set_end() { mBegin = mEnd = nullptr; }

              // http://stackoverflow.com/questions/236129/split-a-string-in-c
            inline void next()
                {
                    for (const char* substart = mEnd == nullptr ? mBegin : mEnd + mDelim.size(), *subend; substart <= mInputEnd; substart = subend + mDelim.size()) {
                        subend = std::search(substart, mInputEnd, mDelim.cbegin(), mDelim.cend());
                        if (substart != subend || mKeepEmpty == Split::KeepEmpty || mKeepEmpty == Split::StripKeepEmpty) {
                            mBegin = substart;
                            mEnd = subend;
                            return;
                        }
                    }
                    set_end();
                }

        }; // class split_iterator

          // ======================================================================

        template <typename S, typename T, typename Extractor> inline std::vector<T> split_into(const S& source, std::string_view delim, Extractor extractor, const char* extractor_name, Split keep_empty = Split::KeepEmpty)
        {
            auto extract = [&](auto chunk) -> T {
                               try {
                                   size_t pos = 0; // g++-9 warn about uninitialized otherwise
                                   const T result = extractor(chunk, &pos);
                                   if (pos != chunk.size())
                                       throw split_error{fmt::format("cannot read {} from \"{}\"", extractor_name, chunk)};
                                   return result;
                               }
                               catch (split_error&) {
                                   throw;
                               }
                               catch (std::exception& err) {
                                   throw split_error{fmt::format("cannot read {} from \"{}\": {}", extractor_name, chunk, err)};
                               }
                           };

            std::vector<T> result;
            std::transform(internal::split_iterator<S>(source, delim, keep_empty), internal::split_iterator<S>(), std::back_inserter(result), extract);
            return result;
        }

        template <typename S, typename T, typename Extractor> inline std::vector<T> split_into(const S& source, Extractor extractor, const char* extractor_name, Split keep_empty = Split::KeepEmpty)
        {
            using namespace std::string_view_literals;
            for (auto delim : {","sv, " "sv, ", "sv, ":"sv, ";"sv}) {
                try {
                    return internal::split_into<S, T>(source, delim, extractor, extractor_name, keep_empty);
                }
                catch (split_error&) {
                }
            }
            throw split_error{fmt::format("cannot extract {}'s from \"{}\"", extractor_name, source)};
        }

    } // namespace internal

    template <typename S> inline std::vector<std::string_view> split(const S& source, std::string_view delim, Split keep_empty = Split::KeepEmpty)
    {
        return {internal::split_iterator<S>(source, delim, keep_empty), internal::split_iterator<S>()};
    }

    template <typename S> inline std::vector<std::string_view> split(const S& source, Split keep_empty = Split::KeepEmpty)
    {
        if (source.find(",") != std::string_view::npos)
            return split(source, ",", keep_empty);
        else if (source.find(" ") != std::string_view::npos)
            return split(source, " ", keep_empty);
        else
            return split(source, "\n", keep_empty);
    }

    template <typename T, typename S> inline std::vector<T> split_into_uint(const S& source, std::string_view delim)
    {
        return internal::split_into<S, T>(source, delim, [](const auto& chunk, size_t* pos) -> T { return T{acmacs::string::from_chars<size_t>(chunk, *pos)}; }, "unsigned", Split::RemoveEmpty);
    }

    template <typename T, typename S> inline std::vector<T> split_into_uint(const S& source)
    {
        return internal::split_into<S, T>(source, [](const auto& chunk, size_t* pos) -> T { return T{acmacs::string::from_chars<size_t>(chunk, *pos)}; }, "unsigned", Split::RemoveEmpty);
    }

    template <typename S> inline std::vector<size_t> split_into_size_t(const S& source, std::string_view delim)
    {
        return internal::split_into<S, size_t>(source, delim, [](const auto& chunk, size_t* pos) -> size_t { return acmacs::string::from_chars<size_t>(chunk, *pos); }, "unsigned", Split::RemoveEmpty);
    }

    template <typename S> inline std::vector<size_t> split_into_size_t(const S& source)
    {
        return internal::split_into<S, size_t>(source, [](const auto& chunk, size_t* pos) -> size_t { return acmacs::string::from_chars<size_t>(chunk, *pos); }, "unsigned", Split::RemoveEmpty);
    }

    template <typename S> inline std::vector<double> split_into_double(const S& source, std::string_view delim)
    {
        return internal::split_into<S, double>(source, delim, [](const auto& chunk, size_t* pos) -> double { return acmacs::string::from_chars<double>(chunk, *pos); }, "double", Split::RemoveEmpty);
    }

    template <typename S> inline std::vector<double> split_into_double(const S& source)
    {
        return internal::split_into<S, double>(source, [](const auto& chunk, size_t* pos) -> double { return acmacs::string::from_chars<double>(chunk, *pos); }, "double", Split::RemoveEmpty);
    }

    // ----------------------------------------------------------------------

    // retrurns empty vector in case source is not camel case, e.g. contains non-letters
    inline std::vector<std::string_view> split_camel_case(std::string_view source)
    {
        std::vector<std::string_view> result;
        bool lower_letter{true};
        size_t start{0};
        for (size_t pos = 0; pos < source.size(); ++pos) {
            if (!std::isalpha(source[pos]))
                return {};
            if (const bool current_upper = std::isupper(source[pos]); current_upper && lower_letter) {
                lower_letter = false;
                if (pos > start)
                    result.emplace_back(&source[start], pos - start);
                start = pos;
            }
            else if (!current_upper && !lower_letter)
                lower_letter = true;
        }
        result.emplace_back(&source[start], source.size() - start);
        return result;
    }

} // namespace acmacs::string

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
