//
// Created by Gary Chen on 1/29/25.
//

#ifndef KEY_VALUE_STORE_TRANSACTION_H
#define KEY_VALUE_STORE_TRANSACTION_H

#include <string>
#include <unordered_map>

template <typename T> class KeyValueStoreTransaction {
public:
  KeyValueStoreTransaction();
  KeyValueStoreTransaction(int type, const std::string& key, const std::optional<T>& value);
  virtual ~KeyValueStoreTransaction() = default;

  bool is_begin() const;
  virtual void rollback(uint32_t& num_records, std::unordered_map<std::string, T>& store);

private:
  int type_;
  std::string key_;
  std::optional<T> value_;
};

template <typename T> KeyValueStoreTransaction<T>::KeyValueStoreTransaction() : type_(0), key_(), value_() {}

template<typename T>
KeyValueStoreTransaction<T>::KeyValueStoreTransaction(int type, const std::string& key, const std::optional<T>& value) : type_(type), key_(key), value_(value) {}

template <typename T> bool KeyValueStoreTransaction<T>::is_begin() const {
  return type_ == 0;  // 0: begin; 1: set; 2: del
}

template <typename T> void KeyValueStoreTransaction<T>::rollback(uint32_t& num_records, std::unordered_map<std::string, T>& store) {
  if (type_ == 1) {
    // rollback set
    auto iter = store.find(key_);
    if (iter == store.end())
      return;

    if (value_.has_value()) {
      store[key_] = value_.value();
    } else {
      store.erase(iter);
      --num_records;
    }
  }
  else if (type_ == 2) {
    // roll back del
    if (value_.has_value()) {
      store[key_] = value_.value();
      ++num_records;
    }
  }
}

#endif //KEY_VALUE_STORE_TRANSACTION_H
