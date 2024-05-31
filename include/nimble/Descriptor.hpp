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
#if defined(CONFIG_BT_NIMBLE_ENABLED) && defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)

#include <string>

#include "nimble/AttributeValue.hpp"
#include "nimble/ConnectionInfo.hpp"
#include "nimble/UUID.hpp"

namespace nimble {

class Characteristic;
class DescriptorCallbacks;

/**
 * @brief A model of a %BLE descriptor.
 */
class Descriptor {
  friend class Characteristic;

public:
  Descriptor(const char *uuid, uint16_t properties, uint16_t max_len, Characteristic *pCharacteristic = nullptr);

  Descriptor(UUID uuid, uint16_t properties, uint16_t max_len, Characteristic *pCharacteristic = nullptr);

  ~Descriptor();

public:
  uint16_t getHandle();
  UUID getUUID();
  std::string toString();
  void setCallbacks(DescriptorCallbacks *pCallbacks);
  Characteristic *getCharacteristic();

  size_t getLength();
  AttributeValue getValue(time_t *timestamp = nullptr);
  std::string getStringValue();

  void setValue(const uint8_t *data, size_t size);
  void setValue(const std::vector<uint8_t> &vec);

  /*********************** Template Functions ************************/

  /**
     * @brief Template to set the characteristic value to <type\>val.
     * @param [in] s The value to set.
     */
  template<typename T>
  void setValue(const T &s) { m_value.setValue<T>(s); }

  /**
     * @brief Template to convert the descriptor data to <type\>.
     * @tparam T The type to convert the data to.
     * @param [in] timestamp (Optional) A pointer to a time_t struct to store the time the value was read.
     * @param [in] skipSizeCheck (Optional) If true it will skip checking if the data size is less than <tt>sizeof(<type\>)</tt>.
     * @return The data converted to <type\> or NULL if skipSizeCheck is false and the data is less than <tt>sizeof(<type\>)</tt>.
     * @details <b>Use:</b> <tt>getValue<type>(&timestamp, skipSizeCheck);</tt>
     */
  template<typename T>
  T getValue(time_t *timestamp = nullptr, bool skipSizeCheck = false) {
    return m_value.getValue<T>(timestamp, skipSizeCheck);
  }

private:
  static int handleGapEvent(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
  void setHandle(uint16_t handle);
  void setCharacteristic(Characteristic *pChar);

private:
  UUID m_uuid;
  uint16_t m_handle;
  DescriptorCallbacks *m_pCallbacks;
  Characteristic *m_pCharacteristic;
  uint8_t m_properties;
  AttributeValue m_value;
  uint8_t m_removed;
};// NimBLEDescriptor

/**
 * @brief Callbacks that can be associated with a %BLE descriptors to inform of events.
 *
 * When a server application creates a %BLE descriptor, we may wish to be informed when there is either
 * a read or write request to the descriptors value.  An application can register a
 * sub-classed instance of this class and will be notified when such an event happens.
 */
class DescriptorCallbacks {
public:
  virtual ~DescriptorCallbacks() = default;
  virtual void onRead(Descriptor *pDescriptor, ConnectionInfo &connInfo);
  virtual void onWrite(Descriptor *pDescriptor, ConnectionInfo &connInfo);
};

}// namespace nimble

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_PERIPHERAL */
