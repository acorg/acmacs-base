#pragma once

#include <vector>

namespace acmacs
{
    using Indexes = std::vector<size_t>;

    class SortedIndexes : public Indexes
    {
     public:
        explicit SortedIndexes(const Indexes& source)
            : Indexes(source) { std::sort(begin(), end()); }
    };

    class ReverseSortedIndexes : public Indexes
    {
     public:
        explicit ReverseSortedIndexes(const Indexes& source) : Indexes(source) { sort(); }

        void add(const Indexes& to_add) { insert(end(), to_add.begin(), to_add.end()); sort(); }

     private:
        static inline bool cmp(size_t i1, size_t i2) { return i1 > i2; }
        void sort() { std::sort(begin(), end(), cmp); erase(std::unique(begin(), end(), cmp), end()); }
    };

    template <typename T> void remove(const ReverseSortedIndexes& indexes, std::vector<T>& data, size_t base_index = 0)
    {
        for (auto index : indexes)
            data.erase(data.begin() + static_cast<ReverseSortedIndexes::difference_type>(index + base_index));
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
