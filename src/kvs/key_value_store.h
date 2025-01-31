/**
 * This is the reference solution file for 18671 assignment 0.
 * Author: Justin Ventura (jmventur@andrew.cmu.edu)
 *
 * NOTE: Keeping template class implementation files in the header file is
 * known as the 'inclusion model' for templates. Keep in mind that doing this is
 * pretty standard, but it can hurt compilation times in large projects. For the
 * sake of the assignment, that's fine.
 */

#ifndef KEY_VALUE_STORE_H
#define KEY_VALUE_STORE_H

#include "store/in_memory_store.h"
#include "kvs/key_value_store_transaction.h"
#include <unordered_map>
#include <stack>

/**
 * @brief The Key Value Store implementation of an In Memory DB.
 */
template <typename T> class KeyValueStore : public InMemoryStore<T> {
public:
  /**
   * @brief Constructor.
   */
  KeyValueStore() {}

  /**
   * @brief Destructor.
   */
  ~KeyValueStore() override {}

  /** MAIN KEY VALUE STORE INTERACTION METHODS; Must be implemented. */
  std::optional<T> Get(std::string_view key) const override;
  void Set(std::string_view key, const T &value) override;
  void Del(std::string_view key) override;

  /** ADDITIONAL METHODS; Must be implemented. */
  std::vector<std::string>
  Keys(std::optional<T> with_value = std::nullopt) const override;
  std::vector<T> Values() const override;
  void Show(uint32_t max_records = 100) const override;
  uint32_t Count(std::optional<T> with_value = std::nullopt) const override;

  /** TRANSACTION SUPPORT METHODS; Unique to this impl */
  void Begin();
  void Commit();
  void Rollback();

private:
  std::unordered_map<std::string, T> store_;
  std::stack<KeyValueStoreTransaction<T>> transactions_;
  int32_t count_begin_ = 0;
};

/** ---------- MAIN PUBLIC METHODS ---------- */

template <typename T>
std::optional<T> KeyValueStore<T>::Get(std::string_view key) const {
  auto iter = store_.find(std::string(key));
  if (iter == store_.end()) {
    return std::nullopt;
  }
  return iter->second;
}

template <typename T>
void KeyValueStore<T>::Set(std::string_view key, const T &value) {
  auto key_str = std::string(key);
  if (!store_.contains(key_str)) {
    ++this->num_records_;

    transactions_.push(KeyValueStoreTransaction<T>(1, key_str, std::nullopt));
  } else {
    transactions_.push(KeyValueStoreTransaction<T>(1, key_str, store_[key_str]));
  }

  store_[key_str] = value;
}

template <typename T> void KeyValueStore<T>::Del(std::string_view key) {
  auto key_str = std::string(key);
  auto iter = store_.find(key_str);
  if (iter == store_.end())
    return;

  transactions_.push(KeyValueStoreTransaction<T>(2, key_str, iter->second));

  store_.erase(iter);
  --this->num_records_;
}

template <typename T> void KeyValueStore<T>::Begin() {
  ++count_begin_;
  transactions_.push(KeyValueStoreTransaction<T>());  //type: begin
}

template <typename T> void KeyValueStore<T>::Commit() {
  while (!transactions_.empty()) {
    transactions_.pop();
  }
}

template <typename T> void KeyValueStore<T>::Rollback() {
  if (count_begin_ == 0 || transactions_.empty())
    return;

  while (true) {
    auto& transaction = transactions_.top();
    if (transaction.is_begin()) {
      transactions_.pop();
      --count_begin_;
      break;
    }
    transaction.rollback(this->num_records_, store_);
    transactions_.pop();
  }
}

template <typename T>
std::vector<std::string>
KeyValueStore<T>::Keys(std::optional<T> with_value) const {
  std::vector<std::string> result;
  for (const auto &[key, val] : store_) {
    if (!with_value.has_value() || with_value.value() == val) {
      result.emplace_back(key);
    }
  }
  return result;
}

template <typename T> std::vector<T> KeyValueStore<T>::Values() const {
  std::vector<T> result;
  result.reserve(this->num_records_);
  for (const auto &[key, val] : store_) {
    result.emplace_back(val);
  }
  return result;
}

template <typename T> void KeyValueStore<T>::Show(uint32_t max_records) const {
  throw std::logic_error("KeyValueStore<T>::Show not implemented.");
}

template <typename T>
uint32_t KeyValueStore<T>::Count(std::optional<T> with_value) const {
  if (!with_value.has_value()) {
    return this->num_records_;
  }

  uint32_t count = 0;
  for (const auto &kv : store_) {
    if (kv.second == with_value.value())
      count++;
  }
  return count;
}

#endif