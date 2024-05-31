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

#include "nimble/Client.hpp"
#include "nimble/RemoteCharacteristic.hpp"
#include "nimble/UUID.hpp"

#include <vector>

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

namespace nimble {

class Client;
class RemoteCharacteristic;

/**
 * @brief A model of a remote %BLE service.
 */
class RemoteService {
public:
  virtual ~RemoteService();

  // Public methods
  std::vector<RemoteCharacteristic *>::iterator begin();
  std::vector<RemoteCharacteristic *>::iterator end();
  RemoteCharacteristic *getCharacteristic(const char *uuid);
  RemoteCharacteristic *getCharacteristic(const UUID &uuid);
  void deleteCharacteristics();
  size_t deleteCharacteristic(const UUID &uuid);
  Client *getClient(void);
  //uint16_t                                  getHandle();
  UUID getUUID(void);
  std::string getValue(const UUID &characteristicUuid);
  bool setValue(const UUID &characteristicUuid,
                const std::string &value);
  std::string toString(void);
  std::vector<RemoteCharacteristic *> *getCharacteristics(bool refresh = false);

private:
  // Private constructor ... never meant to be created by a user application.
  RemoteService(Client *pClient, const struct ble_gatt_svc *service);

  // Friends
  friend class Client;
  friend class RemoteCharacteristic;

  // Private methods
  bool retrieveCharacteristics(const UUID *uuid_filter = nullptr);
  static int characteristicDiscCB(uint16_t conn_handle,
                                  const struct ble_gatt_error *error,
                                  const struct ble_gatt_chr *chr,
                                  void *arg);

  uint16_t getStartHandle();
  uint16_t getEndHandle();
  void releaseSemaphores();

  // Properties

  // We maintain a vector of characteristics owned by this service.
  std::vector<RemoteCharacteristic *> m_characteristicVector;

  Client *m_pClient;
  UUID m_uuid;
  uint16_t m_startHandle;
  uint16_t m_endHandle;
};// NimBLERemoteService

}

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_CENTRAL */
