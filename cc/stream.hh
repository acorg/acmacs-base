#pragma once


#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>

namespace std
{
    // forward for polyfill::make_ostream_joiner
    template <typename Stream, typename V1, typename V2> Stream& operator<<(Stream& out, const std::pair<V1, V2>& aPair);
    template <typename Stream, typename Value> Stream& operator<<(Stream& out, const std::vector<Value>& aCollection);
} // namespace std

#include "acmacs-base/iterator.hh"
#include "acmacs-base/sfinae.hh"

// ----------------------------------------------------------------------
// to work with std::copy, std::transform, operators << have to be declared in the namespace std

template <typename Stream, typename V1, typename V2> inline Stream& operator << (Stream& out, const std::pair<V1, V2>& aPair)
{
    out << '(' << aPair.first << ", " << aPair.second << ')';
    return out;
}

namespace stream_internal
{
    template <typename Stream, typename Iterator> inline Stream& write_to_stream(Stream& out, Iterator first, Iterator last, std::string prefix, std::string suffix, std::string separator)
    {
        out << prefix;
        std::copy(first, last, polyfill::make_ostream_joiner(out, separator));
        out << suffix;
        return out;
    }

    template <typename Stream, typename Collection, typename = std::enable_if<acmacs::sfinae::container_has_iterator<Collection>>>
                      inline Stream& write_to_stream(Stream& out, const Collection& aCollection, std::string prefix, std::string suffix, std::string separator)
    {
        return write_to_stream(out, std::begin(aCollection), std::end(aCollection), prefix, suffix, separator);
    }
}

// ----------------------------------------------------------------------

namespace std
{
    template <typename Stream, typename Value> inline Stream& operator<<(Stream& out, const std::vector<Value>& aCollection)
    {
        return stream_internal::write_to_stream(out, aCollection, "[", "]", ", ");
    }

    // ----------------------------------------------------------------------

    template <typename Stream, typename Value> inline Stream& operator<<(Stream& out, const std::set<Value>& aCollection) { return stream_internal::write_to_stream(out, aCollection, "{", "}", ", "); }

    // ----------------------------------------------------------------------

    template <typename Stream, typename Key, typename Value> inline Stream& operator<<(Stream& out, const std::map<Key, Value>& aCollection)
    {
        out << '{';
        std::transform(std::begin(aCollection), std::end(aCollection), polyfill::make_ostream_joiner(out, ", "), [](const auto& elt) {
            std::ostringstream os;
            os << '<' << elt.first << ">: <" << elt.second << '>';
            return os.str();
        });
        return out << '}';
    }

    // ----------------------------------------------------------------------

    template <typename Stream, typename Value> inline Stream& operator<<(Stream& out, const std::pair<Value*, Value*>& aRange)
    {
        return stream_internal::write_to_stream(out, aRange.first, aRange.second, "[", "]", ", ");
    }

} // namespace std

// ----------------------------------------------------------------------

// template <typename Stream, typename Collection> class to_stream_t
// {
//  public:
//     using value_type = typename Collection::value_type;
//     using const_iterator = typename Collection::const_iterator;
//     using transformer_t = typename std::function<std::string(value_type)>;

//     inline to_stream_t(const_iterator first, const_iterator last, transformer_t transformer) : mFirst(first), mLast(last), mTransformer(transformer) {}

//     inline friend Stream& operator << (Stream& out, const to_stream_t<Stream, Collection>& converter)
//         {
//             if ((converter.mLast - converter.mFirst) > 1)
//                 std::transform(converter.mFirst, converter.mLast, std::ostream_iterator<std::string>(out, " "), converter.mTransformer);
//             else
//                 out << converter.mTransformer(*converter.mFirst);
//             return out;
//         }

//  private:
//     const_iterator mFirst, mLast;
//     transformer_t mTransformer;

// }; // class to_stream

// template <typename Stream, typename Collection> inline auto to_stream(const Collection& c, typename to_stream_t<Stream, Collection>::transformer_t transformer)
// {
//     return to_stream_t<Stream, Collection>(c.begin(), c.end(), transformer);
// }

// template <typename Stream, typename Collection> inline auto to_stream(const Collection& c)
// {
//     return to_stream_t<Stream, Collection>(c.begin(), c.end(), std::to_string);
// }

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
