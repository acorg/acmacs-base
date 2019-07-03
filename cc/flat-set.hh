#pragma once

#include <vector>
#include <algorithm>

// ----------------------------------------------------------------------

namespace acmacs
{
    template <typename T> class flat_set_t
    {
      public:
        flat_set_t() = default;
        flat_set_t(std::initializer_list<T> source) : data_(source) {}

        auto begin() const { return data_.begin(); }
        auto end() const { return data_.end(); }
        auto empty() const { return data_.empty(); }
        auto size() const { return data_.size(); }

        const auto& front() const { return data_.front(); }
        void sort() { std::sort(std::begin(data_), std::end(data_)); }

        template <typename Predicate> void erase_if(Predicate&& pred)
        {
            data_.erase(std::remove_if(std::begin(data_), std::end(data_), std::forward<Predicate>(pred)), std::end(data_));
        }

        void add(const T& elt)
        {
            if (std::find(std::begin(data_), std::end(data_), elt) == std::end(data_))
                data_.push_back(elt);
        }

        void add(T&& elt)
        {
            if (std::find(std::begin(data_), std::end(data_), elt) == std::end(data_))
                data_.push_back(std::move(elt));
        }

        void add_and_sort(const T& elt)
        {
            if (std::find(std::begin(data_), std::end(data_), elt) == std::end(data_)) {
                data_.push_back(elt);
                sort();
            }
        }

        void merge_from(const flat_set_t& source)
        {
            for (const auto& src : source)
                add(src);
        }

      private:
        std::vector<T> data_;
    };

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
