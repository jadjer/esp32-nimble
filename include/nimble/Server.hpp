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

#define NIMBLE_ATT_REMOVE_HIDE 1
#define NIMBLE_ATT_REMOVE_DELETE 2

#include "nimble/Address.hpp"
#include "nimble/Utils.hpp"
#include "nimble/Advertising.hpp"
#include "nimble/ConnectionInfo.hpp"
#include "nimble/Service.hpp"
#include "nimble/UUID.hpp"

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

namespace nimble {

class Device;
class Service;
class Characteristic;
class ServerCallbacks;

/**
 * @brief The model of a %BLE server.
 */
class Server {
  friend class Device;
  friend class Characteristic;
  friend class Service;
  friend class Device;
  friend class Advertising;

public:
  size_t getConnectedCount();
  Service *createService(const char *uuid);
  Service *createService(const UUID &uuid);
  void removeService(Service *service, bool deleteSvc = false);
  void addService(Service *service);
  void setCallbacks(ServerCallbacks *pCallbacks, bool deleteCallbacks = true);

  Advertising *getAdvertising();
  bool startAdvertising(uint32_t duration = 0);
  bool stopAdvertising();

  void start();

  Service *getServiceByUUID(const char *uuid, uint16_t instanceId = 0);
  Service *getServiceByUUID(const UUID &uuid, uint16_t instanceId = 0);
  Service *getServiceByHandle(uint16_t handle);
  int disconnect(uint16_t connID, uint8_t reason = BLE_ERR_REM_USER_CONN_TERM);
  void updateConnParams(uint16_t conn_handle, uint16_t minInterval, uint16_t maxInterval, uint16_t latency, uint16_t timeout);
  void setDataLen(uint16_t conn_handle, uint16_t tx_octets);
  uint16_t getPeerMTU(uint16_t conn_id);
  std::vector<uint16_t> getPeerDevices();
  ConnectionInfo getPeerInfo(size_t index);
  ConnectionInfo getPeerInfo(const Address &address);
  ConnectionInfo getPeerIDInfo(uint16_t id);

  void advertiseOnDisconnect(bool);

private:
  Server();
  ~Server();

  bool m_gattsStarted;
  bool m_advertiseOnDisconnect;
  bool m_svcChanged;
  ServerCallbacks *m_pServerCallbacks;
  bool m_deleteCallbacks;
  uint16_t m_indWait[CONFIG_BT_NIMBLE_MAX_CONNECTIONS];
  std::vector<uint16_t> m_connectedPeersVec;

  //    uint16_t               m_svcChgChrHdl; // Future use

  std::vector<Service *> m_svcVec;
  std::vector<Characteristic *> m_notifyChrVec;

  static int handleGapEvent(struct ble_gap_event *event, void *arg);
  void serviceChanged();
  void resetGATT();
  bool setIndicateWait(uint16_t conn_handle);
  void clearIndicateWait(uint16_t conn_handle);
};// NimBLEServer

/**
 * @brief Callbacks associated with the operation of a %BLE server.
 */
class ServerCallbacks {
public:
  virtual ~ServerCallbacks() = default;

  /**
     * @brief Handle a client connection.
     * This is called when a client connects.
     * @param [in] pServer A pointer to the %BLE server that received the client connection.
     * @param [in] connInfo A reference to a NimBLEConnInfo instance with information
     * about the peer connection parameters.
     */
  virtual void onConnect(Server *pServer, ConnectionInfo &connInfo);

  /**
     * @brief Handle a client disconnection.
     * This is called when a client discconnects.
     * @param [in] pServer A pointer to the %BLE server that received the client disconnection.
     * @param [in] connInfo A reference to a NimBLEConnInfo instance with information
     * about the peer connection parameters.
     * @param [in] reason The reason code for the disconnection.
     */
  virtual void onDisconnect(Server *pServer, ConnectionInfo &connInfo, int reason);

  /**
     * @brief Called when the connection MTU changes.
     * @param [in] MTU The new MTU value.
     * @param [in] connInfo A reference to a NimBLEConnInfo instance with information
     * about the peer connection parameters.
     */
  virtual void onMTUChange(uint16_t MTU, ConnectionInfo &connInfo);

  /**
     * @brief Called when a client requests a passkey for pairing.
     * @return The passkey to be sent to the client.
     */
  virtual uint32_t onPassKeyRequest();

  /**
     * @brief Called when the pairing procedure is complete.
     * @param [in] connInfo A reference to a NimBLEConnInfo instance with information
     * about the peer connection parameters.
     */
  virtual void onAuthenticationComplete(ConnectionInfo &connInfo);

  /**
     * @brief Called when using numeric comparision for pairing.
     * @param [in] pin The pin to compare with the client.
     * @return True to accept the pin.
     */
  virtual bool onConfirmPIN(uint32_t pin);
};// NimBLEServerCallbacks

}// namespace nimble

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_PERIPHERAL */
