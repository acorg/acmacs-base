#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <iterator>
#include <stdexcept>
#include <algorithm>

// ----------------------------------------------------------------------

namespace acmacs::string
{
    enum class Split { RemoveEmpty, KeepEmpty };

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
            inline split_iterator(const S& s, std::string delim, Split keep_empty)
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
                    return {mBegin, static_cast<typename value_type::size_type>(mEnd - mBegin)};
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
            const std::string mDelim;
            const Split mKeepEmpty;
            const char* mBegin;
            const char* mEnd;

            inline void set_end() { mBegin = mEnd = nullptr; }

              // http://stackoverflow.com/questions/236129/split-a-string-in-c
            inline void next()
                {
                    for (const char* substart = mEnd == nullptr ? mBegin : mEnd + mDelim.size(), *subend; substart <= mInputEnd; substart = subend + mDelim.size()) {
                        subend = std::search(substart, mInputEnd, mDelim.cbegin(), mDelim.cend());
                        if (substart != subend || mKeepEmpty == Split::KeepEmpty) {
                            mBegin = substart;
                            mEnd = subend;
                            return;
                        }
                    }
                    set_end();
                }

        }; // class split_iterator

          // ======================================================================

        template <typename S, typename T, typename Extractor> inline std::vector<T> split_into(const S& s, std::string delim, Extractor extractor, const char* extractor_name)
        {
            using namespace std::string_literals;
            auto extract = [&](auto chunk) -> T {
                               try {
                                   size_t pos;
                                   const T result = extractor(chunk, &pos);
                                   if (pos != chunk.size())
                                       throw split_error{"cannot read "s + extractor_name + " from \""s + std::string(chunk) + '"'};
                                   return result;
                               }
                               catch (split_error&) {
                                   throw;
                               }
                               catch (std::exception& err) {
                                   throw split_error{"cannot read "s + extractor_name + " from \""s + std::string(chunk) + "\": " + err.what()};
                               }
                           };

            std::vector<T> result;
            std::transform(internal::split_iterator<S>(s, delim, Split::KeepEmpty), internal::split_iterator<S>(), std::back_inserter(result), extract);
            return result;
        }

    } // namespace internal

    template <typename S> inline std::vector<std::string_view> split(const S& s, std::string delim, Split keep_empty = Split::KeepEmpty)
    {
        return {internal::split_iterator<S>(s, delim, keep_empty), internal::split_iterator<S>()};
    }

    template <typename S> inline std::vector<size_t> split_into_uint(const S& s, std::string delim)
    {
        return internal::split_into<S, size_t>(s, delim, [](const auto& chunk, size_t* pos) -> size_t { return std::stoul(std::string(chunk), pos); }, "unsigned");
    }

    template <typename S> inline std::vector<double> split_into_double(const S& s, std::string delim)
    {
        return internal::split_into<S, double>(s, delim, [](const auto& chunk, size_t* pos) -> double { return std::stod(std::string(chunk), pos); }, "double");
    }

    template <typename S> inline std::vector<double> split_into_double(const S& s)
    {
        using namespace std::string_literals;
        auto extractor = [](const auto& chunk, size_t* pos) -> double { return std::stod(std::string(chunk), pos); };
        for (const char* delim : {",", " ", ", "}) {
            try {
                return internal::split_into<S, double>(s, delim, extractor, "double");
            }
            catch (split_error&) {
            }
        }
        throw split_error{"cannot read double's from \""s + s + "\""};
    }

    // inline std::vector<std::string_view> split(std::string_view s, std::string delim, Split keep_empty = Split::KeepEmpty)
    // {
    //     std::vector<std::string_view> result;
    //     if (! delim.empty()) {
    //         for (auto substart = s.cbegin(), subend = substart; substart <= s.cend(); substart = subend + delim.size()) {
    //             subend = std::search(substart, s.end(), delim.begin(), delim.end());
    //             if (substart != subend || keep_empty == Split::KeepEmpty) {
    //                 result.emplace_back(substart, subend - substart);
    //             }
    //         }
    //     }
    //     else {
    //         result.emplace_back(s.data(), s.size());
    //     }
    //     return result;
    // }

} // namespace acmacs::string

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
