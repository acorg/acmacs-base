#pragma once

#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>

#include "acmacs-base/iterator.hh"

// ----------------------------------------------------------------------

template <typename V1, typename V2> inline std::ostream& operator << (std::ostream& out, const std::pair<V1, V2>& aPair)
{
    return out << '(' << aPair.first << ", " << aPair.second << ')';
}

namespace stream_internal
{
    template <typename Iterator> inline std::ostream& write_to_stream(std::ostream& out, Iterator first, Iterator last, std::string prefix, std::string suffix, std::string separator)
    {
        out << prefix;
        std::copy(first, last, std::make_ostream_joiner(out, separator));
        return out << suffix;
    }

    template <typename Collection> inline std::ostream& write_to_stream(std::ostream& out, const Collection& aCollection, std::string prefix, std::string suffix, std::string separator)
    {
        return write_to_stream(out, std::begin(aCollection), std::end(aCollection), prefix, suffix, separator);
    }
}

// ----------------------------------------------------------------------

template <typename Value> inline std::ostream& operator << (std::ostream& out, const std::vector<Value>& aCollection)
{
    return stream_internal::write_to_stream(out, aCollection, "[", "]", ", ");
}

// ----------------------------------------------------------------------

template <typename Value> inline std::ostream& operator << (std::ostream& out, const std::set<Value>& aCollection)
{
    return stream_internal::write_to_stream(out, aCollection, "{", "}", ", ");
}

// ----------------------------------------------------------------------

template <typename Key, typename Value> inline std::ostream& operator << (std::ostream& out, const std::map<Key, Value>& aCollection)
{
    out << '{';
    std::transform(std::begin(aCollection), std::end(aCollection), std::make_ostream_joiner(out, ", "), [](const auto& elt) { std::ostringstream os; os << '<' << elt.first << ">: <" << elt.second << '>'; return os.str(); });
    // bool sep = false;
    // for (const auto& e: aCollection) {
    //     if (sep)
    //         out << ", ";
    //     else
    //         sep = true;
    //     out << '<' << e.first << ">: <" << e.second << '>';
    // }
    return out << '}';
}

// ----------------------------------------------------------------------

template <typename Collection> class to_stream_t
{
 public:
    using value_type = typename Collection::value_type;
    using const_iterator = typename Collection::const_iterator;
    using transformer_t = typename std::function<std::string(value_type)>;

    inline to_stream_t(const_iterator first, const_iterator last, transformer_t transformer) : mFirst(first), mLast(last), mTransformer(transformer) {}

    inline friend std::ostream& operator << (std::ostream& out, const to_stream_t<Collection>& converter)
        {
            if ((converter.mLast - converter.mFirst) > 1)
                std::transform(converter.mFirst, converter.mLast, std::ostream_iterator<std::string>(out, " "), converter.mTransformer);
            else
                out << converter.mTransformer(*converter.mFirst);
            return out;
        }

 private:
    const_iterator mFirst, mLast;
    transformer_t mTransformer;

}; // class to_stream

template <typename Collection> inline auto to_stream(const Collection& c, typename to_stream_t<Collection>::transformer_t transformer)
{
    return to_stream_t<Collection>(c.begin(), c.end(), transformer);
}

template <typename Collection> inline auto to_stream(const Collection& c)
{
    return to_stream_t<Collection>(c.begin(), c.end(), std::to_string);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
