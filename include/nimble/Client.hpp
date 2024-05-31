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

#include "nimble/Address.hpp"
#include "nimble/AdvertisedDevice.hpp"
#include "nimble/AttributeValue.hpp"
#include "nimble/ConnectionInfo.hpp"
#include "nimble/RemoteService.hpp"
#include "nimble/UUID.hpp"
#include "nimble/Utils.hpp"

#include <string>
#include <vector>

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

namespace nimble {

class RemoteService;
class RemoteCharacteristic;
class ClientCallbacks;
class AdvertisedDevice;

/**
 * @brief A model of a %BLE client.
 */
class Client {
  friend class Device;
  friend class RemoteService;
  friend class ClientCallbacks;

public:
  bool connect(AdvertisedDevice *device, bool deleteAttributes = true);
  bool connect(const Address &address, bool deleteAttributes = true);
  bool connect(bool deleteAttributes = true);
  int disconnect(uint8_t reason = BLE_ERR_REM_USER_CONN_TERM);
  Address getPeerAddress();
  void setPeerAddress(const Address &address);
  int getRssi();
  std::vector<RemoteService *> *getServices(bool refresh = false);
  std::vector<RemoteService *>::iterator begin();
  std::vector<RemoteService *>::iterator end();
  RemoteService *getService(const char *uuid);
  RemoteService *getService(const UUID &uuid);
  void deleteServices();
  size_t deleteService(const UUID &uuid);
  AttributeValue getValue(const UUID &serviceUUID, const UUID &characteristicUUID);
  bool setValue(const UUID &serviceUUID, const UUID &characteristicUUID,
                const AttributeValue &value, bool response = false);
  RemoteCharacteristic *getCharacteristic(uint16_t handle);
  bool isConnected();
  void setClientCallbacks(ClientCallbacks *pClientCallbacks, bool deleteCallbacks = true);
  std::string toString();
  uint16_t getConnId();
  uint16_t getMTU();
  bool secureConnection();
  void setConnectTimeout(uint32_t timeout);
  void setConnectionParams(uint16_t minInterval, uint16_t maxInterval,
                           uint16_t latency, uint16_t timeout,
                           uint16_t scanInterval = 16, uint16_t scanWindow = 16);
  void updateConnParams(uint16_t minInterval, uint16_t maxInterval,
                        uint16_t latency, uint16_t timeout);
  void setDataLen(uint16_t tx_octets);
  bool discoverAttributes();
  ConnectionInfo getConnInfo();
  int getLastError();

private:
  explicit Client(Address const &peerAddress);
  ~Client();

private:
  static int handleGapEvent(struct ble_gap_event *event, void *arg);
  static int serviceDiscoveredCB(uint16_t conn_handle,
                                 const struct ble_gatt_error *error,
                                 const struct ble_gatt_svc *service,
                                 void *arg);
  static void dcTimerCb(ble_npl_event *event);
  bool retrieveServices(const UUID *uuid_filter = nullptr);

private:
  Address m_peerAddress;
  int m_lastErr;
  uint16_t m_conn_id;
  bool m_connEstablished;
  bool m_deleteCallbacks;
  int32_t m_connectTimeout;
  ClientCallbacks *m_pClientCallbacks;
  ble_task_data_t *m_pTaskData;
  ble_npl_callout m_dcTimer;

  std::vector<RemoteService *> m_servicesVector;

private:
  ble_gap_conn_params m_pConnParams;

};// class NimBLEClient

/**
 * @brief Callbacks associated with a %BLE client.
 */
class ClientCallbacks {
public:
  virtual ~ClientCallbacks() = default;

  /**
     * @brief Called after client connects.
     * @param [in] pClient A pointer to the calling client object.
     */
  virtual void onConnect(Client *pClient);

  /**
     * @brief Called when disconnected from the server.
     * @param [in] pClient A pointer to the calling client object.
     * @param [in] reason Contains the reason code for the disconnection.
     */
  virtual void onDisconnect(Client *pClient, int reason);

  /**
     * @brief Called when server requests to update the connection parameters.
     * @param [in] pClient A pointer to the calling client object.
     * @param [in] params A pointer to the struct containing the connection parameters requested.
     * @return True to accept the parameters.
     */
  virtual bool onConnParamsUpdateRequest(Client *pClient, const ble_gap_upd_params *params);

  /**
     * @brief Called when server requests a passkey for pairing.
     * @return The passkey to be sent to the server.
     */
  virtual uint32_t onPassKeyRequest();

  /**
     * @brief Called when the pairing procedure is complete.
     * @param [in] connInfo A reference to a NimBLEConnInfo instance containing the peer info.\n
     * This can be used to check the status of the connection encryption/pairing.
     */
  virtual void onAuthenticationComplete(ConnectionInfo &connInfo);

  /**
     * @brief Called when using numeric comparision for pairing.
     * @param [in] pin The pin to compare with the server.
     * @return True to accept the pin.
     */
  virtual bool onConfirmPIN(uint32_t pin);
};

}// namespace nimble

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_CENTRAL */
