#pragma once

#include <vector>
#include <algorithm>

// ----------------------------------------------------------------------

namespace acmacs
{
    enum class flat_set_sort_afterwards { no, yes };

    template <typename T> class flat_set_t
    {
      public:
        flat_set_t() = default;
        flat_set_t(std::initializer_list<T> source) : data_(source) {}

        auto begin() const { return data_.begin(); }
        auto end() const { return data_.end(); }
        auto empty() const { return data_.empty(); }
        auto size() const { return data_.size(); }
        auto clear() { data_.clear(); }

        const auto& front() const { return data_.front(); }
        void sort() { std::sort(std::begin(data_), std::end(data_)); }

        auto find(const T& key) const { return std::find(std::begin(data_), std::end(data_), key); }
        auto find(const T& key) { return std::find(std::begin(data_), std::end(data_), key); }
        bool exists(const T& key) const { return find(key) != std::end(data_); }

        template <typename Predicate> void erase_if(Predicate&& pred)
        {
            data_.erase(std::remove_if(std::begin(data_), std::end(data_), std::forward<Predicate>(pred)), std::end(data_));
        }

        void add(const T& elt, flat_set_sort_afterwards a_sort = flat_set_sort_afterwards::no)
        {
            if (!exists(elt)) {
                data_.push_back(elt);
                if (a_sort == flat_set_sort_afterwards::yes)
                    sort();
            }
        }

        void add(T&& elt, flat_set_sort_afterwards a_sort = flat_set_sort_afterwards::no)
        {
            if (!exists(elt)) {
                data_.push_back(std::move(elt));
                if (a_sort == flat_set_sort_afterwards::yes)
                    sort();
            }
        }

        void merge_from(const flat_set_t& source, flat_set_sort_afterwards a_sort = flat_set_sort_afterwards::no)
        {
            for (const auto& src : source)
                add(src, flat_set_sort_afterwards::no);
            if (a_sort == flat_set_sort_afterwards::yes)
                sort();
        }

      private:
        std::vector<T> data_;
    };

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
