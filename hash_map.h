// Copyright (c) 2020 Elisaveta Oreshonok <eaoreshonok@edu.hse.ru>
#pragma once

#include <initializer_list>
#include <functional>
#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

template <class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
  using ConstKeyValuePair = std::pair<const KeyType, ValueType>;
  using BucketList = std::list<typename std::list<ConstKeyValuePair>::iterator>;
  using BucketListIterator = typename BucketList::const_iterator;

 public:
  using iterator = typename std::list<ConstKeyValuePair>::iterator;
  using const_iterator = typename std::list<ConstKeyValuePair>::const_iterator;

  HashMap(const Hash &hash = Hash());

  template <class ContainerIterator>
  HashMap(ContainerIterator begin, ContainerIterator end,
          const Hash &hash = Hash());

  HashMap(std::initializer_list<ConstKeyValuePair> initial,
          const Hash &hash = Hash());

  HashMap(const HashMap &other);

  ~HashMap() = default;

  ValueType &operator[](const KeyType key);

  HashMap &operator=(const HashMap &other);

  const ValueType &at(const KeyType key) const;

  void insert(const ConstKeyValuePair elem);

  void erase(const KeyType key);

  iterator find(const KeyType key);

  const_iterator find(const KeyType key) const;

  iterator begin() {
    return element_list_.begin();
  }

  const_iterator begin() const {
    return element_list_.cbegin();
  }

  iterator end() {
    return element_list_.end();
  }

  const_iterator end() const {
    return element_list_.cend();
  }

  bool empty() const {
    return size_ == 0;
  }

  size_t size() const {
    return size_;
  }

  Hash hash_function() const {
    return hasher_;
  }

  void clear();

 private:
  const int kLoadFactor_ = 2;  // min table_size_/cardinality
  const size_t initialSize_ = 2;

  bool IsEqual(const KeyType key, const KeyType other) const {
    return key == other;
  }

  size_t IdxFromKey(const KeyType key) const {
    return hasher_(key) & (table_size_ - 1);
  }

  BucketListIterator RecordInMap(const KeyType key) const;

  void DoubleSize();

  size_t size_ = 0;  // cardinality
  size_t table_size_ = initialSize_;
  std::vector<BucketList> hash_map_ = {};
  std::list<ConstKeyValuePair> element_list_ = {};
  Hash hasher_;
};

template <class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(const Hash &hash) : hasher_(hash) {
  hash_map_.resize(table_size_);
}

template <class KeyType, class ValueType, class Hash>
template <class ContainerIterator>
HashMap<KeyType, ValueType, Hash>::HashMap(ContainerIterator begin,
                                           ContainerIterator end,
                                           const Hash &hash)
    : hasher_(hash) {
  hash_map_.resize(table_size_);
  for (auto element = begin; element != end; ++element) {
    insert(*element);
  }
}

template <class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(const HashMap &other)
    : hasher_(other.hash_function()) {
  hash_map_.resize(table_size_);
  for (auto element : other) {
    insert(element);
  }
}

template <class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(
    std::initializer_list<ConstKeyValuePair> initial, const Hash &hash)
    : hasher_(hash) {
  hash_map_.resize(table_size_);
  for (auto element : initial) {
    insert(element);
  }
}

template <class KeyType, class ValueType, class Hash>
ValueType& HashMap<KeyType, ValueType, Hash>::operator[](const KeyType key) {
  iterator it = find(key);
  if (it != end()) {
    return it->second;
  }
  std::pair<KeyType, ValueType> new_element = {key, ValueType {}};
  insert(new_element);
  return find(key)->second;
}

template <class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>& HashMap<KeyType, ValueType, Hash>::
operator=(const HashMap &other) {
  if (this != &other) {
    size_ = 0;
    hasher_ = other.hash_function();
    table_size_ = initialSize_;
    hash_map_.resize(table_size_);
    clear();
    for (auto element : other) {
      insert(element);
    }
  }
  return *this;
}

template <class KeyType, class ValueType, class Hash>
auto HashMap<KeyType, ValueType, Hash>::find(KeyType key) -> iterator {
  size_t idx = IdxFromKey(key);
  BucketListIterator bucket_list_iterator = RecordInMap(key);
  if (bucket_list_iterator != hash_map_[idx].end()) {
    return *bucket_list_iterator;
  }
  return end();
}

template <class KeyType, class ValueType, class Hash>
auto HashMap<KeyType, ValueType, Hash>::find(const KeyType key) const
-> const_iterator {
  size_t idx = IdxFromKey(key);
  BucketListIterator bucket_list_iterator = RecordInMap(key);
  if (bucket_list_iterator != hash_map_[idx].end()) {
    return *bucket_list_iterator;
  }
  return end();
}

template <class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::clear() {
  size_ = 0;
  table_size_ = initialSize_;
  hash_map_.clear();
  element_list_.clear();
  hash_map_.resize(table_size_);
}

template <class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::erase(const KeyType key) {
  BucketListIterator bucket_list_iterator = RecordInMap(key);
  size_t idx = IdxFromKey(key);
  if (bucket_list_iterator != hash_map_[idx].end()) {
    element_list_.erase(*bucket_list_iterator);
    hash_map_[IdxFromKey(key)].erase(bucket_list_iterator);
    --size_;
  }
}

template <class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::insert(const ConstKeyValuePair elem) {
  if (size_ * kLoadFactor_ >= table_size_) {
    DoubleSize();
  }
  size_t idx = IdxFromKey(elem.first);
  if (find(elem.first) == end()) {
    element_list_.push_front(elem);
    hash_map_[idx].push_back(element_list_.begin());
    ++size_;
  }
}

template <class KeyType, class ValueType, class Hash>
const ValueType &HashMap<KeyType, ValueType, Hash>::
at(const KeyType key) const {
  const_iterator it = find(key);
  if (it != end()) {
    return it->second;
  }
  throw std::out_of_range("Bad request");
}

template <class KeyType, class ValueType, class Hash>
auto HashMap<KeyType, ValueType, Hash>::RecordInMap(const KeyType key) const
-> BucketListIterator {
  size_t idx = IdxFromKey(key);
  BucketListIterator it;
  for (it = hash_map_[idx].begin(); it != hash_map_[idx].end(); ++it) {
    if (IsEqual((*it)->first, key)) {
      return it;
    }
  }
  return it;
}

template <class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::DoubleSize() {
  table_size_ <<= 1;
  size_ = 0;
  hash_map_.clear();
  hash_map_.resize(table_size_);
  for (iterator elem = element_list_.begin(); elem != element_list_.end();
  ++elem) {
    size_t idx = IdxFromKey(elem->first);
    hash_map_[idx].push_back(elem);
    ++size_;
  }
}
