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

#include "host/ble_hs.h"

#include "nimble/AttributeValue.hpp"
#include "nimble/ConnectionInfo.hpp"
#include "nimble/Descriptor.hpp"
#include "nimble/Service.hpp"

#include <string>
#include <vector>

namespace nimble {

typedef enum {
  READ = BLE_GATT_CHR_F_READ,
  READ_ENC = BLE_GATT_CHR_F_READ_ENC,
  READ_AUTHEN = BLE_GATT_CHR_F_READ_AUTHEN,
  READ_AUTHOR = BLE_GATT_CHR_F_READ_AUTHOR,
  WRITE = BLE_GATT_CHR_F_WRITE,
  WRITE_NR = BLE_GATT_CHR_F_WRITE_NO_RSP,
  WRITE_ENC = BLE_GATT_CHR_F_WRITE_ENC,
  WRITE_AUTHEN = BLE_GATT_CHR_F_WRITE_AUTHEN,
  WRITE_AUTHOR = BLE_GATT_CHR_F_WRITE_AUTHOR,
  BROADCAST = BLE_GATT_CHR_F_BROADCAST,
  NOTIFY = BLE_GATT_CHR_F_NOTIFY,
  INDICATE = BLE_GATT_CHR_F_INDICATE
} Property;

class Service;
class Descriptor;
class CharacteristicCallbacks;

/**
 * @brief The model of a %BLE Characteristic.
 *
 * A BLE Characteristic is an identified value container that manages a value. It is exposed by a BLE server and
 * can be read and written to by a %BLE client.
 */
class Characteristic {
public:
  explicit Characteristic(const char *uuid, uint16_t properties = Property::READ | Property::WRITE, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN, Service *pService = nullptr);
  explicit Characteristic(const UUID &uuid, uint16_t properties = Property::READ | Property::WRITE, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN, Service *pService = nullptr);

  ~Characteristic();

public:
  uint16_t getHandle();
  UUID getUUID();
  std::string toString();
  void indicate();
  void indicate(const uint8_t *value, size_t length);
  void indicate(const std::vector<uint8_t> &value);
  void notify(bool is_notification = true, uint16_t conn_handle = BLE_HCI_LE_CONN_HANDLE_MAX + 1);
  void notify(const uint8_t *value, size_t length, bool is_notification = true, uint16_t conn_handle = BLE_HCI_LE_CONN_HANDLE_MAX + 1);
  void notify(const std::vector<uint8_t> &value, bool is_notification = true, uint16_t conn_handle = BLE_HCI_LE_CONN_HANDLE_MAX + 1);
  [[nodiscard]] size_t getSubscribedCount() const;
  void addDescriptor(Descriptor *pDescriptor);
  Descriptor *getDescriptorByUUID(const char *uuid);
  Descriptor *getDescriptorByUUID(const UUID &uuid);
  Descriptor *getDescriptorByHandle(uint16_t handle);
  void removeDescriptor(Descriptor *pDescriptor, bool deleteDsc = false);
  Service *getService();
  uint16_t getProperties();
  AttributeValue getValue(time_t *timestamp = nullptr);
  size_t getDataLength();
  void setValue(const uint8_t *data, size_t size);
  void setValue(const std::vector<uint8_t> &vec);
  void setCallbacks(CharacteristicCallbacks *pCallbacks);

  Descriptor *createDescriptor(const char *uuid, uint32_t properties = NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN);
  Descriptor *createDescriptor(const UUID &uuid, uint32_t properties = NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN);

  CharacteristicCallbacks *getCallbacks();

  /*********************** Template Functions ************************/

  /**
     * @brief Template to set the characteristic value to <type\>val.
     * @param [in] s The value to set.
     */
  template<typename T>
  void setValue(const T &s) { m_value.setValue<T>(s); }

  /**
     * @brief Template to convert the characteristic data to <type\>.
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

  /**
     * @brief Template to send a notification from a class type that has a c_str() and length() method.
     * @tparam T The a reference to a class containing the data to send.
     * @param[in] value The <type\>value to set.
     * @param[in] is_notification if true sends a notification, false sends an indication.
     * @details Only used if the <type\> has a `c_str()` method.
     */
  template<typename T>
  typename std::enable_if<Has_c_str_len<T>::value, void>::type
  notify(const T &value, bool is_notification = true) {
    notify((uint8_t *) value.c_str(), value.length(), is_notification);
  }

  /**
     * @brief Template to send an indication from a class type that has a c_str() and length() method.
     * @tparam T The a reference to a class containing the data to send.
     * @param[in] value The <type\>value to set.
     * @details Only used if the <type\> has a `c_str()` method.
     */
  template<typename T>
  typename std::enable_if<Has_c_str_len<T>::value, void>::type
  indicate(const T &value) {
    indicate((uint8_t *) value.c_str(), value.length());
  }

private:
  friend class Server;
  friend class Service;

  void setService(Service *pService);
  void setSubscribe(struct ble_gap_event *event);
  static int handleGapEvent(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

  UUID m_uuid;
  uint16_t m_handle;
  uint16_t m_properties;
  CharacteristicCallbacks *m_pCallbacks;
  Service *m_pService;
  AttributeValue m_value;
  std::vector<Descriptor *> m_dscVec;
  uint8_t m_removed;

  std::vector<std::pair<uint16_t, uint16_t>> m_subscribedVec;
};// NimBLECharacteristic

/**
 * @brief Callbacks that can be associated with a %BLE characteristic to inform of events.
 *
 * When a server application creates a %BLE characteristic, we may wish to be informed when there is either
 * a read or write request to the characteristic's value. An application can register a
 * sub-classed instance of this class and will be notified when such an event happens.
 */
class CharacteristicCallbacks {
public:
  virtual ~CharacteristicCallbacks() = default;
  virtual void onRead(Characteristic *pCharacteristic, ConnectionInfo &connInfo);
  virtual void onWrite(Characteristic *pCharacteristic, ConnectionInfo &connInfo);
  virtual void onNotify(Characteristic *pCharacteristic);
  virtual void onStatus(Characteristic *pCharacteristic, int code);
  virtual void onSubscribe(Characteristic *pCharacteristic, ConnectionInfo &connInfo, uint16_t subValue);
};

}

#endif /* CONFIG_BT_ENABLED  && CONFIG_BT_NIMBLE_ROLE_PERIPHERAL */
