// Copyright 2024 Pavel Suprunov
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "sdkconfig.h"
#if defined(CONFIG_BT_NIMBLE_ENABLED)

#include <string>
#include <cstring>
#include <ctime>
#include <vector>

#include <host/ble_att.h>
#include <nimble/nimble_npl.h>

#include "nimble/Log.hpp"

#ifndef CONFIG_NIMBLE_CPP_ATT_VALUE_TIMESTAMP_ENABLED
#define CONFIG_NIMBLE_CPP_ATT_VALUE_TIMESTAMP_ENABLED 0
#endif

#if CONFIG_NIMBLE_CPP_ATT_VALUE_TIMESTAMP_ENABLED
#include <ctime>
#endif

#if !defined(CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH)
#define CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH 20
#elif CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH > BLE_ATT_ATTR_MAX_LEN
#error CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH cannot be larger than 512 (BLE_ATT_ATTR_MAX_LEN)
#elif CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH < 1
#error CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH cannot be less than 1; Range = 1 : 512
#endif

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

namespace nimble {

/* Used to determine if the type passed to a template has a c_str() and length() method. */
template<typename T, typename = void, typename = void>
struct Has_c_str_len : std::false_type {};

template<typename T>
struct Has_c_str_len<T, decltype(void(std::declval<T &>().c_str())), decltype(void(std::declval<T &>().length()))> : std::true_type {};

/**
 * @brief A specialized container class to hold BLE attribute values.
 * @details This class is designed to be more memory efficient than using\n
 * standard container types for value storage, while being convertible to\n
 * many different container classes.
 */
class AttributeValue {
  uint8_t *m_attr_value = nullptr;
  uint16_t m_attr_max_len = 0;
  uint16_t m_attr_len = 0;
  uint16_t m_capacity = 0;
#if CONFIG_NIMBLE_CPP_ATT_VALUE_TIMESTAMP_ENABLED
  time_t m_timestamp = 0;
#endif
  void deepCopy(const AttributeValue &source);

public:
  /**
     * @brief Default constructor.
     * @param[in] init_len The initial size in bytes.
     * @param[in] max_len The max size in bytes that the value can be.
     */
  explicit AttributeValue(uint16_t init_len = CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN);

  /**
     * @brief Construct with an initial value from a buffer.
     * @param value A pointer to the initial value to set.
     * @param[in] len The size in bytes of the value to set.
     * @param[in] max_len The max size in bytes that the value can be.
     */
  AttributeValue(const uint8_t *value, uint16_t len, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN);

  /**
     * @brief Construct with an initializer list.
     * @param list An initializer list containing the initial value to set.
     * @param[in] max_len The max size in bytes that the value can be.
     */
  AttributeValue(std::initializer_list<uint8_t> list, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN) : AttributeValue(list.begin(), (uint16_t) list.size(), max_len) {}

  /**
     * @brief Construct with an initial value from a const char string.
     * @param value A pointer to the initial value to set.
     * @param[in] max_len The max size in bytes that the value can be.
     */
  explicit AttributeValue(const char *value, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN) : AttributeValue((uint8_t *) value, (uint16_t) strlen(value), max_len) {}

  /**
     * @brief Construct with an initial value from a std::string.
     * @param str A std::string containing to the initial value to set.
     * @param[in] max_len The max size in bytes that the value can be.
     */
  explicit AttributeValue(std::string const &str, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN) : AttributeValue((uint8_t *) str.data(), (uint16_t) str.length(), max_len) {}

  /**
     * @brief Construct with an initial value from a std::vector<uint8_t>.
     * @param vec A std::vector<uint8_t> containing to the initial value to set.
     * @param[in] max_len The max size in bytes that the value can be.
     */
  explicit AttributeValue(std::vector<uint8_t> const &vec, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN) : AttributeValue(&vec[0], (uint16_t) vec.size(), max_len) {}

  /** @brief Copy constructor */
  AttributeValue(const AttributeValue &source) { deepCopy(source); }

  /** @brief Move constructor */
  AttributeValue(AttributeValue &&source) { *this = std::move(source); }

  /** @brief Destructor */
  ~AttributeValue();

  /** @brief Returns the max size in bytes */
  [[nodiscard]] uint16_t max_size() const { return m_attr_max_len; }

  /** @brief Returns the currently allocated capacity in bytes */
  [[nodiscard]] uint16_t capacity() const { return m_capacity; }

  /** @brief Returns the current length of the value in bytes */
  [[nodiscard]] uint16_t length() const { return m_attr_len; }

  /** @brief Returns the current size of the value in bytes */
  [[nodiscard]] uint16_t size() const { return m_attr_len; }

  /** @brief Returns a pointer to the internal buffer of the value */
  [[nodiscard]] const uint8_t *data() const { return m_attr_value; }

  /** @brief Returns a pointer to the internal buffer of the value as a const char* */
  [[nodiscard]] const char *c_str() const { return (const char *) m_attr_value; }

  /** @brief Iterator begin */
  [[nodiscard]] const uint8_t *begin() const { return m_attr_value; }

  /** @brief Iterator end */
  [[nodiscard]] const uint8_t *end() const { return m_attr_value + m_attr_len; }

#if CONFIG_NIMBLE_CPP_ATT_VALUE_TIMESTAMP_ENABLED
  /** @brief Returns a timestamp of when the value was last updated */
  time_t getTimeStamp() const { return m_timestamp; }

  /** @brief Set the timestamp to the current time */
  void setTimeStamp() { m_timestamp = time(nullptr); }

  /**
     * @brief Set the timestamp to the specified time
     * @param[in] t The timestamp value to set
     */
  void setTimeStamp(time_t t) { m_timestamp = t; }
#else
  time_t getTimeStamp() const { return 0; }
  void setTimeStamp() {}
  void setTimeStamp(time_t t) {}
#endif

  /**
     * @brief Set the value from a buffer
     * @param[in] value A ponter to a buffer containing the value.
     * @param[in] len The length of the value in bytes.
     * @returns True if successful.
     */
  bool setValue(const uint8_t *value, uint16_t len);

  /**
     * @brief Set value to the value of const char*.
     * @param [in] s A ponter to a const char value to set.
     */
  bool setValue(const char *s) {
    return setValue((uint8_t *) s, (uint16_t) strlen(s));
  }

  /**
     * @brief Get a pointer to the value buffer with timestamp.
     * @param[in] timestamp A ponter to a time_t variable to store the timestamp.
     * @returns A pointer to the internal value buffer.
     */
  const uint8_t *getValue(time_t *timestamp);

  /**
     * @brief Append data to the value.
     * @param[in] value A ponter to a data buffer with the value to append.
     * @param[in] len The length of the value to append in bytes.
     * @returns A reference to the appended NimBLEAttValue.
     */
  AttributeValue &append(const uint8_t *value, uint16_t len);

  /*********************** Template Functions ************************/

  /**
     * @brief Template to set value to the value of <type\>val.
     * @param [in] s The <type\>value to set.
     * @details Only used for types without a `c_str()` method.
     */
  template<typename T>
  typename std::enable_if<!Has_c_str_len<T>::value, bool>::type
  setValue(const T &s) {
    return setValue((uint8_t *) &s, sizeof(T));
  }

  /**
     * @brief Template to set value to the value of <type\>val.
     * @param [in] s The <type\>value to set.
     * @details Only used if the <type\> has a `c_str()` method.
     */
  template<typename T>
  typename std::enable_if<Has_c_str_len<T>::value, bool>::type
  setValue(const T &s) {
    return setValue((uint8_t *) s.c_str(), (uint16_t) s.length());
  }

  /**
     * @brief Template to return the value as a <type\>.
     * @tparam T The type to convert the data to.
     * @param [in] timestamp A pointer to a time_t struct to store the time the value was read.
     * @param [in] skipSizeCheck If true it will skip checking if the data size is less than\n
     * <tt>sizeof(<type\>)</tt>.
     * @return The data converted to <type\> or NULL if skipSizeCheck is false and the data is\n
     * less than <tt>sizeof(<type\>)</tt>.
     * @details <b>Use:</b> <tt>getValue<type>(&timestamp, skipSizeCheck);</tt>
     */
  template<typename T>
  T getValue(time_t *timestamp = nullptr, bool skipSizeCheck = false) {
    if (!skipSizeCheck && size() < sizeof(T)) {
      return T();
    }
    return *((T *) getValue(timestamp));
  }

  /*********************** Operators ************************/

  /** @brief Subscript operator */
  uint8_t operator[](int pos) const {
    assert(pos < m_attr_len && "out of range");
    return m_attr_value[pos];
  }

  /** @brief Operator; Get the value as a std::vector<uint8_t>. */
  explicit operator std::vector<uint8_t>() const {
    return {m_attr_value, m_attr_value + m_attr_len};
  }

  /** @brief Operator; Get the value as a std::string. */
  explicit operator std::string() const {
    return std::string((char *) m_attr_value, m_attr_len);
  }

  /** @brief Operator; Get the value as a const uint8_t*. */
  explicit operator const uint8_t *() const { return m_attr_value; }

  /** @brief Operator; Append another NimBLEAttValue. */
  AttributeValue &operator+=(const AttributeValue &source) {
    return append(source.data(), source.size());
  }

  /** @brief Operator; Set the value from a std::string source. */
  AttributeValue &operator=(const std::string &source) {
    setValue((uint8_t *) source.data(), (uint16_t) source.size());
    return *this;
  }

  /** @brief Move assignment operator */
  AttributeValue &operator=(AttributeValue &&source);

  /** @brief Copy assignment operator */
  AttributeValue &operator=(const AttributeValue &source);

  /** @brief Equality operator */
  bool operator==(const AttributeValue &source) {
    return (m_attr_len == source.size()) and memcmp(m_attr_value, source.data(), m_attr_len) == 0;
  }

  /** @brief Inequality operator */
  bool operator!=(const AttributeValue &source) { return !(*this == source); }
};

inline AttributeValue::AttributeValue(uint16_t init_len, uint16_t max_len) {
  assert(init_len > 1);
  assert(init_len < BLE_ATT_ATTR_MAX_LEN);

  m_attr_value = (uint8_t *) calloc(init_len + 1, 1);
  assert(m_attr_value && "No Mem");
  m_attr_max_len = std::min(BLE_ATT_ATTR_MAX_LEN, (int) max_len);
  m_attr_len = 0;
  m_capacity = init_len;
  setTimeStamp(0);
}

inline AttributeValue::AttributeValue(const uint8_t *value, uint16_t len, uint16_t max_len)
    : AttributeValue(len, max_len) {
  memcpy(m_attr_value, value, len);
  m_attr_value[len] = '\0';
  m_attr_len = len;
}

inline AttributeValue::~AttributeValue() {
  if (m_attr_value != nullptr) {
    free(m_attr_value);
  }
}

inline AttributeValue &AttributeValue::operator=(AttributeValue &&source) {
  if (this != &source) {
    free(m_attr_value);

    m_attr_value = source.m_attr_value;
    m_attr_max_len = source.m_attr_max_len;
    m_attr_len = source.m_attr_len;
    m_capacity = source.m_capacity;
    setTimeStamp(source.getTimeStamp());
    source.m_attr_value = nullptr;
  }
  return *this;
}

inline AttributeValue &AttributeValue::operator=(const AttributeValue &source) {
  if (this != &source) {
    deepCopy(source);
  }
  return *this;
}

inline void AttributeValue::deepCopy(const AttributeValue &source) {
  uint8_t *res = (uint8_t *) realloc(m_attr_value, source.m_capacity + 1);
  assert(res && "deepCopy: realloc failed");

  ble_npl_hw_enter_critical();
  m_attr_value = res;
  m_attr_max_len = source.m_attr_max_len;
  m_attr_len = source.m_attr_len;
  m_capacity = source.m_capacity;
  setTimeStamp(source.getTimeStamp());
  memcpy(m_attr_value, source.m_attr_value, m_attr_len + 1);
  ble_npl_hw_exit_critical(0);
}

inline const uint8_t *AttributeValue::getValue(time_t *timestamp) {
  if (timestamp != nullptr) {
#if CONFIG_NIMBLE_CPP_ATT_VALUE_TIMESTAMP_ENABLED
    *timestamp = m_timestamp;
#else
    *timestamp = 0;
#endif
  }
  return m_attr_value;
}

inline bool AttributeValue::setValue(const uint8_t *value, uint16_t len) {
  if (len > m_attr_max_len) {
    NIMBLE_LOGE("NimBLEAttValue", "value exceeds max, len=%u, max=%u", len, m_attr_max_len);
    return false;
  }

  uint8_t *res = m_attr_value;
  if (len > m_capacity) {
    res = (uint8_t *) realloc(m_attr_value, (len + 1));
    m_capacity = len;
  }
  assert(res && "setValue: realloc failed");

#if CONFIG_NIMBLE_CPP_ATT_VALUE_TIMESTAMP_ENABLED
  time_t t = time(nullptr);
#else
  time_t t = 0;
#endif

  ble_npl_hw_enter_critical();
  m_attr_value = res;
  memcpy(m_attr_value, value, len);
  m_attr_value[len] = '\0';
  m_attr_len = len;
  setTimeStamp(t);
  ble_npl_hw_exit_critical(0);
  return true;
}

inline AttributeValue &AttributeValue::append(const uint8_t *value, uint16_t len) {
  if (len < 1) {
    return *this;
  }

  if ((m_attr_len + len) > m_attr_max_len) {
    NIMBLE_LOGE("NimBLEAttValue", "val > max, len=%u, max=%u", len, m_attr_max_len);
    return *this;
  }

  uint8_t *res = m_attr_value;
  uint16_t new_len = m_attr_len + len;
  if (new_len > m_capacity) {
    res = (uint8_t *) realloc(m_attr_value, (new_len + 1));
    m_capacity = new_len;
  }
  assert(res && "append: realloc failed");

#if CONFIG_NIMBLE_CPP_ATT_VALUE_TIMESTAMP_ENABLED
  time_t t = time(nullptr);
#else
  time_t t = 0;
#endif

  ble_npl_hw_enter_critical();
  m_attr_value = res;
  memcpy(m_attr_value + m_attr_len, value, len);
  m_attr_len = new_len;
  m_attr_value[m_attr_len] = '\0';
  setTimeStamp(t);
  ble_npl_hw_exit_critical(0);

  return *this;
}

}// namespace nimble

#endif /*(CONFIG_BT_ENABLED) */
