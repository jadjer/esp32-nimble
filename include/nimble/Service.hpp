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

#include <vector>

#include "nimble/Characteristic.hpp"
#include "nimble/UUID.hpp"

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

namespace nimble {

class Server;

using CharacteristicPtr = Characteristic *;
using Characteristics = std::vector<CharacteristicPtr>;

/**
 * @brief The model of a %BLE service.
 *
 */
class Service {
public:
  explicit Service(const char *uuid);
  explicit Service(const UUID &uuid);
  ~Service();

public:
  static Server *getServer();

public:
  UUID getUUID();
  uint16_t getHandle();
  std::string toString();
  void dump();

public:
  bool start();

public:
  CharacteristicPtr createCharacteristic(const char *uuid, uint32_t properties = Property::READ | Property::WRITE, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN);
  CharacteristicPtr createCharacteristic(const UUID &uuid, uint32_t properties = Property::READ | Property::WRITE, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN);
  CharacteristicPtr createCharacteristic(uint16_t uuid, uint32_t properties = Property::READ | Property::WRITE, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN);

public:
  void addCharacteristic(Characteristic *pCharacteristic);
  void removeCharacteristic(Characteristic *pCharacteristic, bool deleteChr = false);
  CharacteristicPtr getCharacteristic(const char *uuid, uint16_t instanceId = 0);
  CharacteristicPtr getCharacteristic(const UUID &uuid, uint16_t instanceId = 0);
  CharacteristicPtr getCharacteristicByHandle(uint16_t handle);

  Characteristics getCharacteristics();
  Characteristics getCharacteristics(const char *uuid);
  Characteristics getCharacteristics(const UUID &uuid);

private:
  uint16_t m_handle;
  UUID m_uuid;
  ble_gatt_svc_def *m_pSvcDef;
  uint8_t m_removed;
  Characteristics m_characteristics;

};// NimBLEService

}// namespace nimble

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_PERIPHERAL */
