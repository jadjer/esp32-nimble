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
#if defined(CONFIG_BT_NIMBLE_ENABLED) && defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)

#include "nimble/RemoteDescriptor.hpp"
#include "nimble/RemoteService.hpp"

#include "nimble/Log.hpp"
#include <functional>
#include <vector>

namespace nimble {

class RemoteService;
class RemoteDescriptor;

typedef std::function<void(RemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)> notify_callback;

typedef struct {
  const UUID *uuid;
  void *task_data;
} desc_filter_t;

/**
 * @brief A model of a remote %BLE characteristic.
 */
class RemoteCharacteristic {
public:
  ~RemoteCharacteristic();

  // Public member functions
  bool canBroadcast();
  bool canIndicate();
  bool canNotify();
  bool canRead();
  bool canWrite();
  bool canWriteNoResponse();
  std::vector<RemoteDescriptor *>::iterator begin();
  std::vector<RemoteDescriptor *>::iterator end();
  RemoteDescriptor *getDescriptor(const UUID &uuid);
  std::vector<RemoteDescriptor *> *getDescriptors(bool refresh = false);
  void deleteDescriptors();
  size_t deleteDescriptor(const UUID &uuid);
  uint16_t getHandle();
  uint16_t getDefHandle();
  UUID getUUID();
  AttributeValue readValue(time_t *timestamp = nullptr);
  std::string toString();
  RemoteService *getRemoteService();
  AttributeValue getValue(time_t *timestamp = nullptr);
  bool subscribe(bool notifications = true,
                 notify_callback notifyCallback = nullptr,
                 bool response = true);
  bool unsubscribe(bool response = true);
  bool writeValue(const uint8_t *data,
                  size_t length,
                  bool response = false);
  bool writeValue(const std::vector<uint8_t> &v, bool response = false);
  bool writeValue(const char *s, bool response = false);

  /*********************** Template Functions ************************/

  /**
     * @brief Template to set the remote characteristic value to <type\>val.
     * @param [in] s The value to write.
     * @param [in] response True == request write response.
     * @details Only used for non-arrays and types without a `c_str()` method.
     */
  template<typename T>
  typename std::enable_if<!std::is_array<T>::value && !Has_c_str_len<T>::value, bool>::type
  writeValue(const T &s, bool response = false) {
    return writeValue((uint8_t *) &s, sizeof(T), response);
  }

  /**
     * @brief Template to set the remote characteristic value to <type\>val.
     * @param [in] s The value to write.
     * @param [in] response True == request write response.
     * @details Only used if the <type\> has a `c_str()` method.
     */
  template<typename T>
  typename std::enable_if<Has_c_str_len<T>::value, bool>::type
  writeValue(const T &s, bool response = false) {
    return writeValue((uint8_t *) s.c_str(), s.length(), response);
  }

  /**
     * @brief Template to convert the remote characteristic data to <type\>.
     * @tparam T The type to convert the data to.
     * @param [in] timestamp A pointer to a time_t struct to store the time the value was read.
     * @param [in] skipSizeCheck If true it will skip checking if the data size is less than <tt>sizeof(<type\>)</tt>.
     * @return The data converted to <type\> or NULL if skipSizeCheck is false and the data is
     * less than <tt>sizeof(<type\>)</tt>.
     * @details <b>Use:</b> <tt>getValue<type>(&timestamp, skipSizeCheck);</tt>
     */
  template<typename T>
  T getValue(time_t *timestamp = nullptr, bool skipSizeCheck = false) {
    if (!skipSizeCheck && m_value.size() < sizeof(T))
      return T();
    return *((T *) m_value.getValue(timestamp));
  }

  /**
     * @brief Template to convert the remote characteristic data to <type\>.
     * @tparam T The type to convert the data to.
     * @param [in] timestamp A pointer to a time_t struct to store the time the value was read.
     * @param [in] skipSizeCheck If true it will skip checking if the data size is less than <tt>sizeof(<type\>)</tt>.
     * @return The data converted to <type\> or NULL if skipSizeCheck is false and the data is
     * less than <tt>sizeof(<type\>)</tt>.
     * @details <b>Use:</b> <tt>readValue<type>(&timestamp, skipSizeCheck);</tt>
     */
  template<typename T>
  T readValue(time_t *timestamp = nullptr, bool skipSizeCheck = false) {
    AttributeValue value = readValue();
    if (!skipSizeCheck && value.size() < sizeof(T))
      return T();
    return *((T *) value.getValue(timestamp));
  }

private:
  RemoteCharacteristic(RemoteService *pRemoteservice, const struct ble_gatt_chr *chr);

  friend class Client;
  friend class RemoteService;
  friend class RemoteDescriptor;

  // Private member functions
  bool setNotify(uint16_t val, notify_callback notifyCallback = nullptr, bool response = true);
  bool retrieveDescriptors(const UUID *uuid_filter = nullptr);
  static int onReadCB(uint16_t conn_handle, const struct ble_gatt_error *error,
                      struct ble_gatt_attr *attr, void *arg);
  static int onWriteCB(uint16_t conn_handle, const struct ble_gatt_error *error,
                       struct ble_gatt_attr *attr, void *arg);
  static int descriptorDiscCB(uint16_t conn_handle, const struct ble_gatt_error *error,
                              uint16_t chr_val_handle, const struct ble_gatt_dsc *dsc,
                              void *arg);
  static int nextCharCB(uint16_t conn_handle, const struct ble_gatt_error *error,
                        const struct ble_gatt_chr *chr, void *arg);

  // Private properties
  UUID m_uuid;
  uint8_t m_charProp;
  uint16_t m_handle;
  uint16_t m_defHandle;
  uint16_t m_endHandle;
  RemoteService *m_pRemoteService;
  AttributeValue m_value;
  notify_callback m_notifyCallback;

  // We maintain a vector of descriptors owned by this characteristic.
  std::vector<RemoteDescriptor *> m_descriptorVector;
};// NimBLERemoteCharacteristic

}

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_CENTRAL */
