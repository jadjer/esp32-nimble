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

#include "nimble/AttributeValue.hpp"
#include "nimble/RemoteCharacteristic.hpp"
#include "nimble/UUID.hpp"

namespace nimble {

class RemoteCharacteristic;
/**
 * @brief A model of remote %BLE descriptor.
 */
class RemoteDescriptor {
  friend class RemoteCharacteristic;

public:
  uint16_t getHandle();
  RemoteCharacteristic *getRemoteCharacteristic();
  UUID getUUID();
  AttributeValue readValue();
  std::string toString();
  bool writeValue(const uint8_t *data, size_t length, bool response = false);
  bool writeValue(const std::vector<uint8_t> &v, bool response = false);
  bool writeValue(const char *s, bool response = false);

  /*********************** Template Functions ************************/

  /**
     * @brief Template to set the remote descriptor value to <type\>val.
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
     * @brief Template to set the remote descriptor value to <type\>val.
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
     * @brief Template to convert the remote descriptor data to <type\>.
     * @tparam T The type to convert the data to.
     * @param [in] skipSizeCheck If true it will skip checking if the data size is less than <tt>sizeof(<type\>)</tt>.
     * @return The data converted to <type\> or NULL if skipSizeCheck is false and the data is
     * less than <tt>sizeof(<type\>)</tt>.
     * @details <b>Use:</b> <tt>readValue<type>(skipSizeCheck);</tt>
     */
  template<typename T>
  T readValue(bool skipSizeCheck = false) {
    AttributeValue value = readValue();
    if (!skipSizeCheck && value.size() < sizeof(T)) {
      return T();
    }
    return *((T *) value.data());
  }

private:
  RemoteDescriptor(RemoteCharacteristic *pRemoteCharacteristic, const struct ble_gatt_dsc *dsc);
  static int onWriteCB(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg);
  static int onReadCB(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg);

private:
  uint16_t m_handle;
  UUID m_uuid;
  RemoteCharacteristic *m_pRemoteCharacteristic;
};

}// namespace nimble

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_CENTRAL */
