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

#include "nimble/Characteristic.hpp"
#include "nimble/Server.hpp"
#include "nimble/UUID.hpp"

namespace nimble {

class Server;
class Characteristic;


/**
 * @brief The model of a %BLE service.
 *
 */
class Service {
public:
  Service(const char* uuid);
  Service(const UUID &uuid);
    ~Service();

    Server *         getServer();

    UUID getUUID();
    uint16_t              getHandle();
    std::string           toString();
    void                  dump();

    bool                  start();

    Characteristic * createCharacteristic(const char* uuid,
                                              uint32_t properties =
                                              NIMBLE_PROPERTY::READ |
                                              NIMBLE_PROPERTY::WRITE,
                                              uint16_t max_len = BLE_ATT_ATTR_MAX_LEN);

    Characteristic * createCharacteristic(const UUID &uuid,
                                               uint32_t properties =
                                               NIMBLE_PROPERTY::READ |
                                               NIMBLE_PROPERTY::WRITE,
                                               uint16_t max_len = BLE_ATT_ATTR_MAX_LEN);

    void                  addCharacteristic(Characteristic * pCharacteristic);
    void                  removeCharacteristic(Characteristic * pCharacteristic, bool deleteChr = false);
    Characteristic * getCharacteristic(const char* uuid, uint16_t instanceId = 0);
    Characteristic * getCharacteristic(const UUID &uuid, uint16_t instanceId = 0);
    Characteristic * getCharacteristicByHandle(uint16_t handle);

    std::vector<Characteristic *> getCharacteristics();
    std::vector<Characteristic *> getCharacteristics(const char* uuid);
    std::vector<Characteristic *> getCharacteristics(const UUID &uuid);


private:

    friend class Server;
    friend class          NimBLEDevice;

    uint16_t              m_handle;
    UUID m_uuid;
    ble_gatt_svc_def*     m_pSvcDef;
    uint8_t               m_removed;
    std::vector<Characteristic *> m_chrVec;

}; // NimBLEService

}

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_PERIPHERAL */
