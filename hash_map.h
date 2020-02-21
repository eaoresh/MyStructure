// Copyright (c) 2020 Elisaveta Oreshonok <eaoreshonok@edu.hse.ru>

#include <initializer_list>
#include <functional>
#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

template <class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
  using element = std::pair<KeyType, ValueType>;
  using const_element = std::pair<const KeyType, ValueType>;
  using bucket = std::list<typename std::list<const_element>::iterator>;

 public:
  using iterator = typename std::list<const_element>::iterator;
  using const_iterator = typename std::list<const_element>::const_iterator;

  HashMap(const Hash &hash = Hash());

  template <class ContainerIterator>
  HashMap(ContainerIterator begin, ContainerIterator end,
          const Hash &hash = Hash());

  HashMap(std::initializer_list<element> initial, const Hash &hash = Hash());

  HashMap(const HashMap &other);

  ~HashMap() = default;

  ValueType &operator[](const KeyType key);

  HashMap &operator=(const HashMap &other);

  const ValueType &at(const KeyType key) const;

  void insert(const const_element elem);

  void erase(const KeyType key);

  iterator find(const KeyType key);
  const_iterator find(const KeyType key) const;

  iterator begin() { return element_list_.begin(); }
  const_iterator begin() const { return element_list_.begin(); }

  iterator end() { return element_list_.end(); }
  const_iterator end() const { return element_list_.end(); }

  bool empty() const { return size_ == 0; }

  size_t size() const { return size_; }

  Hash hash_function() const { return hasher_; }

  void clear();

 private:
  const int kLoadFactor = 2;  // min table_size_/cardinality

  bool is_equal(const KeyType key, const KeyType other) const {
    return key == other;
  }

  size_t idx_from_key(const KeyType key) const {
    return hasher_(key) & (table_size_ - 1);
  }

  void Expansion();

  size_t size_;  // cardinality
  size_t table_size_;
  std::vector<bucket> hash_map_ = {};
  std::list<const_element> element_list_ = {};
  Hash hasher_;
};

template <class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(const Hash &hash) : hasher_(hash) {
  size_ = 0;
  table_size_ = 2;
  hash_map_.resize(table_size_, bucket {});
}

template <class KeyType, class ValueType, class Hash>
template <class ContainerIterator>
HashMap<KeyType, ValueType, Hash>::HashMap(ContainerIterator begin,
                                           ContainerIterator end,
                                           const Hash &hash)
    : hasher_(hash) {
  size_ = 0;
  table_size_ = 2;
  hash_map_.resize(table_size_, bucket {});
  for (auto element = begin; element != end; ++element) {
    insert(*element);
  }
}

template <class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(const HashMap &other)
    : hasher_(other.hash_function()) {
  size_ = 0;
  table_size_ = 2;
  hash_map_.resize(table_size_, bucket {});
  for (auto element : other) {
    insert(element);
  }
}

template <class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(
    std::initializer_list<element> initial, const Hash &hash)
    : hasher_(hash) {
  size_ = 0;
  table_size_ = 2;
  hash_map_.resize(table_size_, bucket {});
  for (auto element : initial) {
    insert(element);
  }
}

template <class KeyType, class ValueType, class Hash>
ValueType& HashMap<KeyType, ValueType, Hash>::operator[](
    const KeyType key) {
  if (find(key) != end()) {
    return find(key)->second;
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
    table_size_ = 2;
    hash_map_.resize(table_size_, bucket {});
    clear();
    for (auto element : other) {
      insert(element);
    }
  }
  return *this;
}

template <class KeyType, class ValueType, class Hash>
typename std::list<std::pair<const KeyType, ValueType>>::iterator
HashMap<KeyType, ValueType, Hash>::find(const KeyType key) {
  size_t idx = idx_from_key(key);
  for (auto record : hash_map_[idx]) {
    if (is_equal(record->first, key)) {
      return record;
    }
  }
  return end();
}

template <class KeyType, class ValueType, class Hash>
typename std::list<std::pair<const KeyType, ValueType>>::const_iterator
HashMap<KeyType, ValueType, Hash>::find(const KeyType key) const {
  size_t idx = idx_from_key(key);
  for (auto record : hash_map_[idx]) {
    if (is_equal(record->first, key)) {
      return record;
    }
  }
  return end();
}

template <class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::clear() {
  size_ = 0;
  table_size_ = 2;
  hash_map_.clear();
  element_list_.clear();
  hash_map_.resize(table_size_, bucket {});
}

template <class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::erase(const KeyType key) {
  if (find(key) != end()) {
    size_t idx = idx_from_key(key);
    typename bucket::iterator it;
    for (it = hash_map_[idx].begin(); it != hash_map_[idx].end(); ++it) {
      if ((*it)->first == key)
        break;
    }
    element_list_.erase(find(key));
    hash_map_[idx].erase(it);
    --size_;
  }
}

template <class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::insert(const const_element elem) {
  if (size_ * kLoadFactor >= table_size_)
    Expansion();
  size_t idx = idx_from_key(elem.first);
  if (find(elem.first) == end()) {
    element_list_.push_front(elem);
    hash_map_[idx].push_back(element_list_.begin());
    ++size_;
  }
}

template <class KeyType, class ValueType, class Hash>
const ValueType &HashMap<KeyType, ValueType, Hash>::
    at(const KeyType key) const {
  if (find(key) != end()) {
    return find(key)->second;
  }
  throw std::out_of_range("Bad request");
}

template <class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::Expansion() {
  table_size_ <<= 1;
  size_ = 0;
  hash_map_.clear();
  hash_map_.resize(table_size_);
  iterator elem;
  for (elem = element_list_.begin(); elem != element_list_.end(); ++elem) {
    size_t idx = idx_from_key(elem->first);
    hash_map_[idx].push_back(elem);
    ++size_;
  }
}
