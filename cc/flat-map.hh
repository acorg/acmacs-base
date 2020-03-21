#pragma once

#include <vector>

#include "acmacs-base/fmt.hh"
#include "acmacs-base/range-v3.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
    namespace detail
    {
        template <typename Key, typename Value> class map_base_t
        {
          public:
            using entry_type = std::pair<Key, Value>;
            using const_iterator = typename std::vector<entry_type>::const_iterator;

            // map_base_t() = default;
            virtual ~map_base_t() = default;

            bool empty() const noexcept { return data_.empty(); }
            constexpr const auto& data() const noexcept { return data_; }

            template <typename Range> void collect(Range&& rng, bool check_result = true)
            {
                data_ = rng | ranges::to<std::vector>;
                sorted_ = false;
                if (check_result && data_.size() > 1)
                    check();
            }

            template <typename EKey, typename EValue> auto& emplace(EKey&& key, EValue&& value)
            {
                sorted_ = false;
                return data_.emplace_back(std::forward<EKey>(key), std::forward<EValue>(value));
            }

          protected:
            constexpr auto& data() noexcept { return data_; }
            // constexpr bool sorted() const noexcept { return sorted_; }

            template <typename FindKey> const_iterator find_first(const FindKey& key) const noexcept
            {
                sort();
                return std::lower_bound(std::begin(data_), std::end(data_), key, [](const auto& e1, const auto& k2) { return e1.first < k2; });
            }

            virtual void check() const {}

            void sort() const noexcept
            {
                if (!sorted_) {
                    ranges::sort(data_, [](const auto& e1, const auto& e2) { return e1.first < e2.first; });
                    sorted_ = true;
                }
            }

          private:
            mutable std::vector<entry_type> data_;
            mutable bool sorted_{false};

        };
    } // namespace detail

    // ----------------------------------------------------------------------

    // see seqdb.cc Seqdb::hash_index() for sample usage
    template <typename Key, typename Value> class map_with_duplicating_keys_t : public detail::map_base_t<Key, Value>
    {
      public:
        using const_iterator = typename detail::map_base_t<Key, Value>::const_iterator;

        template <typename FindKey> std::pair<const_iterator, const_iterator> find(const FindKey& key) const noexcept
        {
            const auto first = this->find_first(key);
            // first may point to the wrong key, if key is not in the map
            return {first, std::find_if(first, std::end(this->data()), [&key](const auto& en) { return en.first != key; })};
        }

    };

    // ----------------------------------------------------------------------

    class map_with_unique_keys_error : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

    // see seqdb.cc Seqdb::seq_id_index() for sample usage
    template <typename Key, typename Value> class map_with_unique_keys_t : public detail::map_base_t<Key, Value>
    {
      public:
        using const_iterator = typename detail::map_base_t<Key, Value>::const_iterator;

        template <typename FindKey> const Value* find(const FindKey& key) const noexcept
        {
            if (const auto first = this->find_first(key); first->first == key)
                return &first->second;
            else
                return nullptr;
        }

      protected:
        void check() const override
        {
            this->sort();
            for (auto cur = std::next(std::begin(this->data())); cur != std::end(this->data()); ++cur) {
                if (cur->first == std::prev(cur)->first)
                    throw map_with_unique_keys_error{"duplicating keys within map_with_unique_keys_t"};
            }
        }

    };

    // ----------------------------------------------------------------------

    template <typename Key, typename Value> class small_map_with_unique_keys_t
    {
      public:
        using entry_type = std::pair<Key, Value>;
        using const_iterator = typename std::vector<entry_type>::const_iterator;

        small_map_with_unique_keys_t() = default;
        // template <typename Iter> small_map_with_unique_keys_t(Iter first, Iter last) : data_(first, last) {}
        small_map_with_unique_keys_t(std::initializer_list<entry_type> init) : data_{init} {}

        constexpr const auto& data() const noexcept { return data_; }
        auto begin() const noexcept { return data_.begin(); }
        auto end() const noexcept { return data_.end(); }
        auto empty() const noexcept { return data_.empty(); }
        auto size() const noexcept { return data_.size(); }

        template <typename K> auto find(const K& key) const noexcept
        {
            return std::find_if(std::begin(data_), std::end(data_), [&key](const auto& en) { return en.first == key; });
        }

        template <typename K> auto find(const K& key) noexcept
        {
            return std::find_if(std::begin(data_), std::end(data_), [&key](const auto& en) { return en.first == key; });
        }

        template <typename K, typename Callback> void find_then(const K& key, Callback callback) const noexcept
        {
            if (const auto& en = find(key); en != std::end(data_))
                callback(en->second);
        }

        template <typename K> Value& get(const K& key)
        {
            if (const auto found = find(key); found != std::end(data_))
                return found->second;
            throw std::out_of_range{fmt::format("acmacs::small_map_with_unique_keys_t::at(): no key: {}", key)};
        }

        template <typename K> const Value& get(const K& key) const
        {
            if (const auto found = find(key); found != std::end(data_))
                return found->second;
            throw std::out_of_range{fmt::format("acmacs::small_map_with_unique_keys_t::at(): no key: {}", key)};
        }

        template <typename K, typename V> const Value& get_or(const K& key, const V& dflt) const
        {
            if (const auto found = find(key); found != std::end(data_))
                return found->second;
            else
                return dflt;
        }

        template <typename K, typename V> auto& emplace_or_replace(const K& key, const V& value)
        {
            if (auto found = find(key); found != end()) {
                found->second = Value{value};
                return *found;
            }
            else
                return data_.emplace_back(Key{key}, Value{value});
        }

        template <typename K, typename V = Value> auto& emplace_not_replace(const K& key, const V& value = V{})
        {
            if (auto found = find(key); found != end())
                return *found;
            else
                return data_.emplace_back(Key{key}, Value{value});
        }

      private:
        std::vector<entry_type> data_;

    }; // small_map_with_unique_keys_t<Key, Value>

    // ----------------------------------------------------------------------

    template <typename Key, typename Value> class flat_map_t
    {
      public:
        using key_type = Key;
        using value_type = Value;
        using entry_type = std::pair<Key, Value>;

        flat_map_t() = default;
        template <typename Iter> flat_map_t(Iter first, Iter last) : data_(first, last) {}
        flat_map_t(std::initializer_list<entry_type> init) : data_{init} {}

        auto begin() const { return data_.begin(); }
        auto end() const { return data_.end(); }
        auto empty() const { return data_.empty(); }
        auto size() const { return data_.size(); }
        const auto& operator[](size_t index) const { return data_[index]; }
        const auto& back() const { return data_.back(); }
        auto& back() { return data_.back(); }
        void sort_by_key()
        {
            std::sort(std::begin(data_), std::end(data_), [](const auto& e1, const auto& e2) { return e1.first < e2.first; });
        }
        void sort_by_value()
        {
            std::sort(std::begin(data_), std::end(data_), [](const auto& e1, const auto& e2) { return e1.second < e2.second; });
        }
        void sort_by_value_reverse()
        {
            std::sort(std::begin(data_), std::end(data_), [](const auto& e1, const auto& e2) { return e1.second > e2.second; });
        }

        template <typename K> auto find(const K& key) const
        {
            return std::find_if(std::begin(data_), std::end(data_), [&key](const auto& en) { return en.first == key; });
        }

        template <typename K> auto find(const K& key)
        {
            return std::find_if(std::begin(data_), std::end(data_), [&key](const auto& en) { return en.first == key; });
        }
        template <typename K> bool exists(const K& key) const { return find(key) != std::end(data_); }

        Value& at(const Key& key)
        {
            if (const auto found = find(key); found != std::end(data_))
                return found->second;
            throw std::out_of_range{fmt::format("acmacs::flat_map_t::at(): no key: {}", key)};
        }

        const Value& at(const Key& key) const
        {
            if (const auto found = find(key); found != std::end(data_))
                return found->second;
            throw std::out_of_range{fmt::format("acmacs::flat_map_t::at(): no key: {}", key)};
        }

        const Value* at_ptr(const Key& key) const
        {
            if (const auto found = find(key); found != std::end(data_))
                return &found->second;
            else
                return nullptr;
        }

        auto& emplace(const Key& key, const Value& value) { return data_.emplace_back(key, value); }
        auto& emplace(Key&& key, Value&& value) { return data_.emplace_back(std::move(key), std::move(value)); }

        auto& emplace_or_replace(const Key& key, const Value& value)
        {
            if (auto found = find(key); found != std::end(data_)) {
                found->second = value;
                return *found;
            }
            else
                return emplace(key, value);
        }

        auto& emplace_not_replace(const Key& key, const Value& value)
        {
            if (auto found = find(key); found != std::end(data_))
                return *found;
            else
                return emplace(key, value);
        }

        auto& emplace_not_replace(const Key& key)
        {
            if (auto found = find(key); found != std::end(data_))
                return *found;
            else
                return emplace(key, Value{});
        }

        void reserve(size_t sz) { data_.reserve(sz); }

      private:
        std::vector<entry_type> data_;
    };

} // namespace acmacs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
