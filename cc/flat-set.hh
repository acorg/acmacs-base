#pragma once

#include <vector>

// ----------------------------------------------------------------------

namespace acmacs
{
    template <typename T> class flat_set_t
    {
      public:
        flat_set_t() = default;

        auto begin() const { return data_.begin(); }
        auto end() const { return data_.end(); }
        auto empty() const { return data_.empty(); }
        const auto& front() const { return data_.front(); }
        void sort() { std::sort(std::begin(data_), std::end(data_)); }

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
