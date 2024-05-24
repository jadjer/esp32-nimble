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

#include "nimconfig.h"
#if defined(CONFIG_BT_ENABLED) && defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)

#include "NimBLEClient.h"
#include "NimBLEUUID.h"
#include "NimBLERemoteCharacteristic.h"

#include <vector>

class NimBLEClient;
class NimBLERemoteCharacteristic;


/**
 * @brief A model of a remote %BLE service.
 */
class NimBLERemoteService {
public:
    virtual ~NimBLERemoteService();

    // Public methods
    std::vector<NimBLERemoteCharacteristic*>::iterator begin();
    std::vector<NimBLERemoteCharacteristic*>::iterator end();
    NimBLERemoteCharacteristic*               getCharacteristic(const char* uuid);
    NimBLERemoteCharacteristic*               getCharacteristic(const NimBLEUUID &uuid);
    void                                      deleteCharacteristics();
    size_t                                    deleteCharacteristic(const NimBLEUUID &uuid);
    NimBLEClient*                             getClient(void);
    //uint16_t                                  getHandle();
    NimBLEUUID                                getUUID(void);
    std::string                               getValue(const NimBLEUUID &characteristicUuid);
    bool                                      setValue(const NimBLEUUID &characteristicUuid,
                                                       const std::string &value);
    std::string                               toString(void);
    std::vector<NimBLERemoteCharacteristic*>* getCharacteristics(bool refresh = false);

private:
    // Private constructor ... never meant to be created by a user application.
    NimBLERemoteService(NimBLEClient* pClient, const struct ble_gatt_svc *service);

    // Friends
    friend class NimBLEClient;
    friend class NimBLERemoteCharacteristic;

    // Private methods
    bool                retrieveCharacteristics(const NimBLEUUID *uuid_filter = nullptr);
    static int          characteristicDiscCB(uint16_t conn_handle,
                                             const struct ble_gatt_error *error,
                                             const struct ble_gatt_chr *chr,
                                             void *arg);

    uint16_t            getStartHandle();
    uint16_t            getEndHandle();
    void                releaseSemaphores();

    // Properties

    // We maintain a vector of characteristics owned by this service.
    std::vector<NimBLERemoteCharacteristic*> m_characteristicVector;

    NimBLEClient*       m_pClient;
    NimBLEUUID          m_uuid;
    uint16_t            m_startHandle;
    uint16_t            m_endHandle;
}; // NimBLERemoteService

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_CENTRAL */
