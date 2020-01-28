#pragma once

#include <vector>

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace acmacs
{
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
        void sort_by_key() { std::sort(std::begin(data_), std::end(data_), [](const auto& e1, const auto& e2) { return e1.first < e2.first; }); }
        void sort_by_value() { std::sort(std::begin(data_), std::end(data_), [](const auto& e1, const auto& e2) { return e1.second < e2.second; }); }
        void sort_by_value_reverse() { std::sort(std::begin(data_), std::end(data_), [](const auto& e1, const auto& e2) { return e1.second > e2.second; }); }

        template <typename K> auto find(const K& key) const { return std::find_if(std::begin(data_), std::end(data_), [&key](const auto& en) { return en.first == key; }); }
        template <typename K> auto find(const K& key) { return std::find_if(std::begin(data_), std::end(data_), [&key](const auto& en) { return en.first == key; }); }
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
