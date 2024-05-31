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

#include <host/ble_gap.h>

#include "nimble/Address.hpp"

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

namespace nimble {

/**
 * @brief Connection information.
 */
class ConnectionInfo {
  friend class Server;
  friend class Client;
  friend class Characteristic;
  friend class Descriptor;

  ble_gap_conn_desc m_desc{};
  ConnectionInfo() { m_desc = {}; }
  explicit ConnectionInfo(ble_gap_conn_desc desc) { m_desc = desc; }

public:
  /** @brief Gets the over-the-air address of the connected peer */
  [[nodiscard]] Address getAddress() const { return Address(m_desc.peer_ota_addr); }

  /** @brief Gets the ID address of the connected peer */
  [[nodiscard]] Address getIdAddress() const { return Address(m_desc.peer_id_addr); }

  /** @brief Gets the connection handle of the connected peer */
  [[nodiscard]] uint16_t getConnHandle() const { return m_desc.conn_handle; }

  /** @brief Gets the connection interval for this connection (in 1.25ms units) */
  [[nodiscard]] uint16_t getConnInterval() const { return m_desc.conn_itvl; }

  /** @brief Gets the supervision timeout for this connection (in 10ms units) */
  [[nodiscard]] uint16_t getConnTimeout() const { return m_desc.supervision_timeout; }

  /** @brief Gets the allowable latency for this connection (unit = number of intervals) */
  [[nodiscard]] uint16_t getConnLatency() const { return m_desc.conn_latency; }

  /** @brief Gets the maximum transmission unit size for this connection (in bytes) */
  [[nodiscard]] uint16_t getMTU() const { return ble_att_mtu(m_desc.conn_handle); }

  /** @brief Check if we are in the master role in this connection */
  [[nodiscard]] bool isMaster() const { return (m_desc.role == BLE_GAP_ROLE_MASTER); }

  /** @brief Check if we are in the slave role in this connection */
  [[nodiscard]] bool isSlave() const { return (m_desc.role == BLE_GAP_ROLE_SLAVE); }

  /** @brief Check if we are connected to a bonded peer */
  [[nodiscard]] bool isBonded() const { return (m_desc.sec_state.bonded == 1); }

  /** @brief Check if the connection in encrypted */
  [[nodiscard]] bool isEncrypted() const { return (m_desc.sec_state.encrypted == 1); }

  /** @brief Check if the the connection has been authenticated */
  [[nodiscard]] bool isAuthenticated() const { return (m_desc.sec_state.authenticated == 1); }

  /** @brief Gets the key size used to encrypt the connection */
  [[nodiscard]] uint8_t getSecKeySize() const { return m_desc.sec_state.key_size; }
};

}
