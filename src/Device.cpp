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

#include "sdkconfig.h"
#if defined(CONFIG_BT_NIMBLE_ENABLED)

#include <esp_bt.h>
#include <esp_err.h>
#include <host/ble_hs.h>
#include <host/ble_hs_pvcy.h>
#include <host/util/util.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <nvs_flash.h>
#include <services/gap/ble_svc_gap.h>
#include <services/gatt/ble_svc_gatt.h>

#include "nimble/Device.hpp"
#include "nimble/Log.hpp"
#include "nimble/Scan.hpp"
#include "nimble/Utils.hpp"

namespace nimble {

static const char *LOG_TAG = "Device";

bool Device::m_isInitialized = false;
bool Device::m_isSynced = false;

#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
Scan *Device::m_pScan = nullptr;
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
Server *Device::m_pServer = nullptr;
#endif

uint32_t Device::m_passkey = 123456;

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
Advertising *Device::m_bleAdvertising = nullptr;
#endif

gap_event_handler Device::m_customGapHandler = nullptr;

ble_gap_event_listener Device::m_listener;

#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)
std::list<Client *> Device::m_cList;
#endif

std::list<Address> Device::m_ignoreList;
std::vector<Address> Device::m_whiteList;

uint8_t Device::m_own_addr_type = BLE_OWN_ADDR_PUBLIC;

#ifdef CONFIG_BTDM_BLE_SCAN_DUPL
uint16_t Device::m_scanDuplicateSize = CONFIG_BTDM_SCAN_DUPL_CACHE_SIZE;
uint8_t Device::m_scanFilterMode = CONFIG_BTDM_SCAN_DUPL_TYPE;
#endif

/**
 * @brief Create a new instance of a server.
 * @return A new instance of the server.
 */
#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
/* STATIC */ Server *Device::createServer() {
  if (Device::m_pServer == nullptr) {
    Device::m_pServer = new Server();
    ble_gatts_reset();
    ble_svc_gap_init();
    ble_svc_gatt_init();
  }

  return m_pServer;
}// createServer

/**
 * @brief Get the instance of the server.
 * @return A pointer to the server instance.
 */
/* STATIC */ Server *Device::getServer() {
  return m_pServer;
}// getServer
#endif// #if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
/**
 * @brief Get the instance of the advertising object.
 * @return A pointer to the advertising object.
 */
Advertising *Device::getAdvertising() {
  if (m_bleAdvertising == nullptr) {
    m_bleAdvertising = new Advertising();
  }
  return m_bleAdvertising;
}

/**
 * @brief Convenience function to begin advertising.
 * @param [in] duration The duration in milliseconds to advertise for, default = forever.
 * @return True if advertising started successfully.
 */
bool Device::startAdvertising(uint32_t duration) {
  return getAdvertising()->start(duration);
}// startAdvertising

/**
 * @brief Convenience function to stop all advertising.
 * @return True if advertising stopped successfully.
 */
bool Device::stopAdvertising() {
  return getAdvertising()->stop();
}// stopAdvertising
#endif// #if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)

/**
 * @brief Retrieve the Scan object that we use for scanning.
 * @return The scanning object reference.  This is a singleton object.  The caller should not
 * try and release/delete it.
 */
#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
/* STATIC */
Scan *Device::getScan() {
  if (m_pScan == nullptr) {
    m_pScan = new Scan();
  }
  return m_pScan;
}// getScan
#endif// #if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)

/**
 * @brief Creates a new client object and maintains a list of all client objects
 * each client can connect to 1 peripheral device.
 * @param [in] peerAddress An optional peer address that is copied to the new client
 * object, allows for calling NimBLEClient::connect(bool) without a device or address parameter.
 * @return A reference to the new client object.
 */
#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)
/* STATIC */
Client *Device::createClient(Address peerAddress) {
  if (m_cList.size() >= CONFIG_BT_NIMBLE_MAX_CONNECTIONS) {
    NIMBLE_LOGW(LOG_TAG, "Number of clients exceeds Max connections. Cur=%d Max=%d", m_cList.size(), CONFIG_BT_NIMBLE_MAX_CONNECTIONS);
  }

  auto pClient = new Client(peerAddress);
  m_cList.push_back(pClient);

  return pClient;
}// createClient

/**
 * @brief Delete the client object and remove it from the list.\n
 * Checks if it is connected or trying to connect and disconnects/stops it first.
 * @param [in] pClient A pointer to the client object.
 */
/* STATIC */
bool Device::deleteClient(Client *pClient) {
  if (pClient == nullptr) {
    return false;
  }

  // Set the connection established flag to false to stop notifications
  // from accessing the attribute vectors while they are being deleted.
  pClient->m_connEstablished = false;
  int rc = 0;

  if (pClient->isConnected()) {
    rc = pClient->disconnect();
    if (rc != 0 && rc != BLE_HS_EALREADY && rc != BLE_HS_ENOTCONN) {
      return false;
    }

    while (pClient->isConnected()) {
      taskYIELD();
    }
    // Since we set the flag to false the app will not get a callback
    // in the disconnect event so we call it here for good measure.
    pClient->m_pClientCallbacks->onDisconnect(pClient, BLE_ERR_CONN_TERM_LOCAL);

  } else if (pClient->m_pTaskData != nullptr) {
    rc = ble_gap_conn_cancel();
    if (rc != 0 && rc != BLE_HS_EALREADY) {
      return false;
    }
    while (pClient->m_pTaskData != nullptr) {
      taskYIELD();
    }
  }

  m_cList.remove(pClient);
  delete pClient;

  return true;
}// deleteClient

/**
 * @brief Get the list of created client objects.
 * @return A pointer to the list of clients.
 */
/* STATIC */
std::list<Client *> *Device::getClientList() {
  return &m_cList;
}// getClientList

/**
 * @brief Get the number of created client objects.
 * @return Number of client objects created.
 */
/* STATIC */
size_t Device::getClientListSize() {
  return m_cList.size();
}// getClientList

/**
 * @brief Get a reference to a client by connection ID.
 * @param [in] conn_id The client connection ID to search for.
 * @return A pointer to the client object with the spcified connection ID.
 */
/* STATIC */
Client *Device::getClientByID(uint16_t conn_id) {
  for (auto it : m_cList) {
    if (it->getConnId() == conn_id) {
      return it;
    }
  }
  assert(0);
  return nullptr;
}// getClientByID

/**
 * @brief Get a reference to a client by peer address.
 * @param [in] peer_addr The address of the peer to search for.
 * @return A pointer to the client object with the peer address.
 */
/* STATIC */
Client *Device::getClientByPeerAddress(const Address &peer_addr) {
  for (auto it : m_cList) {
    if (it->getPeerAddress().equals(peer_addr)) {
      return it;
    }
  }
  return nullptr;
}// getClientPeerAddress

/**
 * @brief Finds the first disconnected client in the list.
 * @return A pointer to the first client object that is not connected to a peer.
 */
/* STATIC */
Client *Device::getDisconnectedClient() {
  for (auto it : m_cList) {
    if (!it->isConnected()) {
      return it;
    }
  }
  return nullptr;
}// getDisconnectedClient

#endif// #if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)

/**
 * @brief Set the transmission power.
 * @param [in] powerLevel The power level to set, can be one of:
 * *   ESP_PWR_LVL_N12 = 0, Corresponding to -12dbm
 * *   ESP_PWR_LVL_N9  = 1, Corresponding to  -9dbm
 * *   ESP_PWR_LVL_N6  = 2, Corresponding to  -6dbm
 * *   ESP_PWR_LVL_N3  = 3, Corresponding to  -3dbm
 * *   ESP_PWR_LVL_N0  = 4, Corresponding to   0dbm
 * *   ESP_PWR_LVL_P3  = 5, Corresponding to  +3dbm
 * *   ESP_PWR_LVL_P6  = 6, Corresponding to  +6dbm
 * *   ESP_PWR_LVL_P9  = 7, Corresponding to  +9dbm
 * @param [in] powerType The BLE function to set the power level for, can be one of:
 * *   ESP_BLE_PWR_TYPE_CONN_HDL0  = 0,  For connection handle 0
 * *   ESP_BLE_PWR_TYPE_CONN_HDL1  = 1,  For connection handle 1
 * *   ESP_BLE_PWR_TYPE_CONN_HDL2  = 2,  For connection handle 2
 * *   ESP_BLE_PWR_TYPE_CONN_HDL3  = 3,  For connection handle 3
 * *   ESP_BLE_PWR_TYPE_CONN_HDL4  = 4,  For connection handle 4
 * *   ESP_BLE_PWR_TYPE_CONN_HDL5  = 5,  For connection handle 5
 * *   ESP_BLE_PWR_TYPE_CONN_HDL6  = 6,  For connection handle 6
 * *   ESP_BLE_PWR_TYPE_CONN_HDL7  = 7,  For connection handle 7
 * *   ESP_BLE_PWR_TYPE_CONN_HDL8  = 8,  For connection handle 8
 * *   ESP_BLE_PWR_TYPE_ADV        = 9,  For advertising
 * *   ESP_BLE_PWR_TYPE_SCAN       = 10, For scan
 * *   ESP_BLE_PWR_TYPE_DEFAULT    = 11, For default, if not set other, it will use default value
 */
/* STATIC */
void Device::setPower(esp_power_level_t powerLevel, esp_ble_power_type_t powerType) {
  NIMBLE_LOGD(LOG_TAG, ">> setPower: %d (type: %d)", powerLevel, powerType);

  esp_err_t errRc = esp_ble_tx_power_set(powerType, powerLevel);
  if (errRc != ESP_OK) {
    NIMBLE_LOGE(LOG_TAG, "esp_ble_tx_power_set: rc=%d", errRc);
  }

  NIMBLE_LOGD(LOG_TAG, "<< setPower");
}// setPower

/**
 * @brief Get the transmission power.
 * @param [in] powerType The power level to set, can be one of:
 * *   ESP_BLE_PWR_TYPE_CONN_HDL0  = 0,  For connection handle 0
 * *   ESP_BLE_PWR_TYPE_CONN_HDL1  = 1,  For connection handle 1
 * *   ESP_BLE_PWR_TYPE_CONN_HDL2  = 2,  For connection handle 2
 * *   ESP_BLE_PWR_TYPE_CONN_HDL3  = 3,  For connection handle 3
 * *   ESP_BLE_PWR_TYPE_CONN_HDL4  = 4,  For connection handle 4
 * *   ESP_BLE_PWR_TYPE_CONN_HDL5  = 5,  For connection handle 5
 * *   ESP_BLE_PWR_TYPE_CONN_HDL6  = 6,  For connection handle 6
 * *   ESP_BLE_PWR_TYPE_CONN_HDL7  = 7,  For connection handle 7
 * *   ESP_BLE_PWR_TYPE_CONN_HDL8  = 8,  For connection handle 8
 * *   ESP_BLE_PWR_TYPE_ADV        = 9,  For advertising
 * *   ESP_BLE_PWR_TYPE_SCAN       = 10, For scan
 * *   ESP_BLE_PWR_TYPE_DEFAULT    = 11, For default, if not set other, it will use default value
 * @return the power level currently used by the type specified.
 */
/* STATIC */
int8_t Device::getPower(esp_ble_power_type_t powerType) {
  switch (esp_ble_tx_power_get(powerType)) {
  case ESP_PWR_LVL_N12:
    return -12;
  case ESP_PWR_LVL_N9:
    return -9;
  case ESP_PWR_LVL_N6:
    return -6;
  case ESP_PWR_LVL_N3:
    return -3;
  case ESP_PWR_LVL_N0:
    return 0;
  case ESP_PWR_LVL_P3:
    return 3;
  case ESP_PWR_LVL_P6:
    return 6;
  case ESP_PWR_LVL_P9:
    return 9;
  default:
    return BLE_HS_ADV_TX_PWR_LVL_AUTO;
  }
}// getPower

/**
 * @brief Get our device address.
 * @return A NimBLEAddress object of our public address if we have one,
 * if not then our current random address.
 */
/* STATIC*/
Address Device::getAddress() {
  ble_addr_t addr = {BLE_ADDR_PUBLIC, {0}};

  if (BLE_HS_ENOADDR == ble_hs_id_copy_addr(BLE_ADDR_PUBLIC, addr.val, nullptr)) {
    NIMBLE_LOGD(LOG_TAG, "Public address not found, checking random");
    addr.type = BLE_ADDR_RANDOM;
    ble_hs_id_copy_addr(BLE_ADDR_RANDOM, addr.val, nullptr);
  }

  return {addr};
}// getAddress

/**
 * @brief Return a string representation of the address of this device.
 * @return A string representation of this device address.
 */
/* STATIC */
std::string Device::toString() {
  return getAddress().toString();
}// toString

/**
 * @brief Setup local mtu that will be used to negotiate mtu during request from client peer.
 * @param [in] mtu Value to set local mtu:
 * * This should be larger than 23 and lower or equal to BLE_ATT_MTU_MAX = 527.
 */
/* STATIC */
int Device::setMTU(uint16_t mtu) {
  NIMBLE_LOGD(LOG_TAG, ">> setLocalMTU: %d", mtu);

  int rc = ble_att_set_preferred_mtu(mtu);

  if (rc != 0) {
    NIMBLE_LOGE(LOG_TAG, "Could not set local mtu value to: %d", mtu);
  }

  NIMBLE_LOGD(LOG_TAG, "<< setLocalMTU");
  return rc;
}// setMTU

/**
 * @brief Get local MTU value set.
 * @return The current preferred MTU setting.
 */
/* STATIC */
uint16_t Device::getMTU() {
  return ble_att_preferred_mtu();
}

#ifdef CONFIG_BTDM_BLE_SCAN_DUPL
/**
 * @brief Set the duplicate filter cache size for filtering scanned devices.
 * @param [in] cacheSize The number of advertisements filtered before the cache is reset.\n
 * Range is 10-1000, a larger value will reduce how often the same devices are reported.
 * @details Must only be called before calling Device::init.
 */
/*STATIC*/
void Device::setScanDuplicateCacheSize(uint16_t cacheSize) {
  if (initialized) {
    NIMBLE_LOGE(LOG_TAG, "Cannot change scan cache size while initialized");
    return;
  } else if (cacheSize > 1000 || cacheSize < 10) {
    NIMBLE_LOGE(LOG_TAG, "Invalid scan cache size; min=10 max=1000");
    return;
  }

  m_scanDuplicateSize = cacheSize;
}

/**
 * @brief Set the duplicate filter mode for filtering scanned devices.
 * @param [in] mode One of three possible options:
 * * CONFIG_BTDM_SCAN_DUPL_TYPE_DEVICE (0) (default)\n
     Filter by device address only, advertisements from the same address will be reported only once.
 * * CONFIG_BTDM_SCAN_DUPL_TYPE_DATA (1)\n
     Filter by data only, advertisements with the same data will only be reported once,\n
     even from different addresses.
 * * CONFIG_BTDM_SCAN_DUPL_TYPE_DATA_DEVICE (2)\n
     Filter by address and data, advertisements from the same address will be reported only once,\n
     except if the data in the advertisement has changed, then it will be reported again.
 * @details Must only be called before calling Device::init.
 */
/*STATIC*/
void Device::setScanFilterMode(uint8_t mode) {
  if (initialized) {
    NIMBLE_LOGE(LOG_TAG, "Cannot change scan duplicate type while initialized");
    return;
  } else if (mode > 2) {
    NIMBLE_LOGE(LOG_TAG, "Invalid scan duplicate type");
    return;
  }

  m_scanFilterMode = mode;
}
#endif// CONFIG_BTDM_BLE_SCAN_DUPL

#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL) || defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
/**
 * @brief Gets the number of bonded peers stored
 */
/*STATIC*/
int Device::getNumBonds() {
  ble_addr_t peer_id_addrs[MYNEWT_VAL(BLE_STORE_MAX_BONDS)];
  int num_peers, rc;

  rc = ble_store_util_bonded_peers(&peer_id_addrs[0], &num_peers, MYNEWT_VAL(BLE_STORE_MAX_BONDS));
  if (rc != 0) {
    return 0;
  }

  return num_peers;
}

/**
 * @brief Deletes all bonding information.
 */
/*STATIC*/
void Device::deleteAllBonds() {
  ble_store_clear();
}

/**
 * @brief Deletes a peer bond.
 * @param [in] address The address of the peer with which to delete bond info.
 * @returns true on success.
 */
/*STATIC*/
bool Device::deleteBond(const Address &address) {
  ble_addr_t delAddr;
  memcpy(&delAddr.val, address.getNative(), 6);
  delAddr.type = address.getType();

  int rc = ble_gap_unpair(&delAddr);
  if (rc != 0) {
    return false;
  }

  return true;
}

/**
 * @brief Checks if a peer device is bonded.
 * @param [in] address The address to check for bonding.
 * @returns true if bonded.
 */
/*STATIC*/
bool Device::isBonded(const Address &address) {
  ble_addr_t peer_id_addrs[MYNEWT_VAL(BLE_STORE_MAX_BONDS)];
  int num_peers, rc;

  rc = ble_store_util_bonded_peers(&peer_id_addrs[0], &num_peers, MYNEWT_VAL(BLE_STORE_MAX_BONDS));
  if (rc != 0) {
    return false;
  }

  for (int i = 0; i < num_peers; i++) {
    Address storedAddr(peer_id_addrs[i]);
    if (storedAddr == address) {
      return true;
    }
  }

  return false;
}

/**
 * @brief Get the address of a bonded peer device by index.
 * @param [in] index The index to retrieve the peer address of.
 * @returns NimBLEAddress of the found bonded peer or nullptr if not found.
 */
/*STATIC*/
Address Device::getBondedAddress(int index) {
  ble_addr_t peer_id_addrs[MYNEWT_VAL(BLE_STORE_MAX_BONDS)];
  int num_peers, rc;

  rc = ble_store_util_bonded_peers(&peer_id_addrs[0], &num_peers, MYNEWT_VAL(BLE_STORE_MAX_BONDS));
  if (rc != 0) {
    return nullptr;
  }

  if (index > num_peers || index < 0) {
    return nullptr;
  }

  return {peer_id_addrs[index]};
}
#endif

/**
 * @brief Checks if a peer device is whitelisted.
 * @param [in] address The address to check for in the whitelist.
 * @returns true if the address is in the whitelist.
 */
/*STATIC*/
bool Device::onWhiteList(const Address &address) {
  for (auto &it : m_whiteList) {
    if (it == address) {
      return true;
    }
  }

  return false;
}

/**
 * @brief Add a peer address to the whitelist.
 * @param [in] address The address to add to the whitelist.
 * @returns true if successful.
 */
/*STATIC*/
bool Device::whiteListAdd(const Address &address) {
  if (Device::onWhiteList(address)) {
    return true;
  }

  m_whiteList.push_back(address);
  std::vector<ble_addr_t> wlVec;
  wlVec.reserve(m_whiteList.size());

  for (auto &it : m_whiteList) {
    ble_addr_t wlAddr;
    memcpy(&wlAddr.val, it.getNative(), 6);
    wlAddr.type = it.getType();
    wlVec.push_back(wlAddr);
  }

  int rc = ble_gap_wl_set(&wlVec[0], wlVec.size());
  if (rc != 0) {
    NIMBLE_LOGE(LOG_TAG, "Failed adding to whitelist rc=%d", rc);
    return false;
  }

  return true;
}

/**
 * @brief Remove a peer address from the whitelist.
 * @param [in] address The address to remove from the whitelist.
 * @returns true if successful.
 */
/*STATIC*/
bool Device::whiteListRemove(const Address &address) {
  if (!Device::onWhiteList(address)) {
    return true;
  }

  std::vector<ble_addr_t> wlVec;
  wlVec.reserve(m_whiteList.size());

  for (auto &it : m_whiteList) {
    if (it != address) {
      ble_addr_t wlAddr;
      memcpy(&wlAddr.val, it.getNative(), 6);
      wlAddr.type = it.getType();
      wlVec.push_back(wlAddr);
    }
  }

  int rc = ble_gap_wl_set(&wlVec[0], wlVec.size());
  if (rc != 0) {
    NIMBLE_LOGE(LOG_TAG, "Failed removing from whitelist rc=%d", rc);
    return false;
  }

  // Don't remove from the list unless NimBLE returned success
  for (auto it = m_whiteList.begin(); it < m_whiteList.end(); ++it) {
    if ((*it) == address) {
      m_whiteList.erase(it);
      break;
    }
  }

  return true;
}

/**
 * @brief Gets the count of addresses in the whitelist.
 * @returns The number of addresses in the whitelist.
 */
/*STATIC*/
size_t Device::getWhiteListCount() {
  return m_whiteList.size();
}

/**
 * @brief Gets the address at the vector index.
 * @param [in] index The vector index to retrieve the address from.
 * @returns the NimBLEAddress at the whitelist index or nullptr if not found.
 */
/*STATIC*/
Address Device::getWhiteListAddress(size_t index) {
  if (index > m_whiteList.size()) {
    NIMBLE_LOGE(LOG_TAG, "Invalid index; %u", index);
    return nullptr;
  }
  return m_whiteList[index];
}

/**
 * @brief Host reset, we pass the message so we don't make calls until resynced.
 * @param [in] reason The reason code for the reset.
 */
/* STATIC */
void Device::onReset(int reason) {
  if (not m_isSynced) {
    return;
  }

  m_isSynced = false;

  NIMBLE_LOGC(LOG_TAG, "Resetting state; reason=%d, %s", reason, Utils::returnCodeToString(reason));

#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
  if (not m_isInitialized) {
    return;
  }

  if (m_pScan == nullptr) {
    return;
  }

  m_pScan->onHostReset();
#endif
}// onReset

/**
 * @brief Host resynced with controller, all clear to make calls to the stack.
 */
/* STATIC */
void Device::onSync() {
  NIMBLE_LOGI(LOG_TAG, "NimBle host synced.");
  // This check is needed due to potentially being called multiple times in succession
  // If this happens, the call to scan start may get stuck or cause an advertising fault.
  if (m_isSynced) {
    return;
  }

  /* Make sure we have proper identity address set (public preferred) */
  int rc = ble_hs_util_ensure_addr(0);
  assert(rc == 0);

  // Yield for housekeeping before returning to operations.
  // Occasionally triggers exception without.
  taskYIELD();

  m_isSynced = true;

  if (m_isInitialized) {
#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
    if (m_pScan != nullptr) {
      m_pScan->onHostSync();
    }
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
    if (m_bleAdvertising != nullptr) {
      m_bleAdvertising->onHostSync();
    }
#endif
  }
}// onSync

/**
 * @brief The main host task.
 */
/* STATIC */
void Device::hostTask(void *param) {
  (void) param;

  NIMBLE_LOGI(LOG_TAG, "BLE Host Task Started");

  /* This function will return only when nimble_port_stop() is executed */
  nimble_port_run();

  nimble_port_freertos_deinit();
}// host_task

/**
 * @brief Initialize the %BLE environment.
 * @param [in] deviceName The device name of the device.
 */
/* STATIC */
void Device::init(const std::string &deviceName) {
  if (not m_isInitialized) {
    int rc = 0;

    esp_err_t errRc = ESP_OK;

    errRc = nvs_flash_init();
    if (errRc == ESP_ERR_NVS_NO_FREE_PAGES || errRc == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      errRc = nvs_flash_init();
    }
    ESP_ERROR_CHECK(errRc);

    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

    nimble_port_init();

    // Setup callbacks for host events
    ble_hs_cfg.reset_cb = Device::onReset;
    ble_hs_cfg.sync_cb = Device::onSync;

    // Set initial security capabilities
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_bonding = 0;
    ble_hs_cfg.sm_mitm = 0;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_our_key_dist = 1;
    ble_hs_cfg.sm_their_key_dist = 3;

    ble_hs_cfg.store_status_cb = ble_store_util_status_rr; /*TODO: Implement handler for this*/

    // Set the device name.
    rc = ble_svc_gap_device_name_set(deviceName.c_str());
    assert(rc == 0);

    ble_store_config_init();

    nimble_port_freertos_init(Device::hostTask);
  }

  // Wait for host and controller to sync before returning and accepting new tasks
  while (not m_isSynced) {
    taskYIELD();
  }

  m_isInitialized = true;// Set the initialization flag to ensure we are only initialized once.
}// init

/**
 * @brief Shutdown the NimBLE stack/controller.
 * @param [in] clearAll If true, deletes all server/advertising/scan/client objects after deinitializing.
 * @note If clearAll is true when called, any references to the created objects become invalid.
 */
/* STATIC */
void Device::deinit(bool clearAll) {
  int ret = nimble_port_stop();
  if (ret == 0) {
    nimble_port_deinit();

    m_isInitialized = false;
    m_isSynced = false;

    if (clearAll) {
#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
      if (Device::m_pServer != nullptr) {
        delete Device::m_pServer;
        Device::m_pServer = nullptr;
      }
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
      if (Device::m_bleAdvertising != nullptr) {
        delete Device::m_bleAdvertising;
        Device::m_bleAdvertising = nullptr;
      }
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
      if (Device::m_pScan != nullptr) {
        delete Device::m_pScan;
        Device::m_pScan = nullptr;
      }
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)
      for (auto &it : m_cList) {
        deleteClient(it);
        m_cList.clear();
      }
#endif

      m_ignoreList.clear();
    }
  }
}// deinit

/**
 * @brief Set the BLEDevice's name
 * @param [in] deviceName The device name of the device.
 */
/* STATIC */
void Device::setDeviceName(const std::string &deviceName) {
  ble_svc_gap_device_name_set(deviceName.c_str());
}// setDeviceName

/**
 * @brief Check if the initialization is complete.
 * @return true if initialized.
 */
/*STATIC*/
bool Device::isInitialized() {
  return m_isInitialized;
}// getInitialized

/**
 * @brief Set the authorization mode for this device.
 * @param bonding If true we allow bonding, false no bonding will be performed.
 * @param mitm If true we are capable of man in the middle protection, false if not.
 * @param sc If true we will perform secure connection pairing, false we will use legacy pairing.
 */
/*STATIC*/
void Device::setSecurityAuth(bool bonding, bool mitm, bool sc) {
  NIMBLE_LOGD(LOG_TAG, "Setting bonding: %d, mitm: %d, sc: %d", bonding, mitm, sc);
  ble_hs_cfg.sm_bonding = bonding;
  ble_hs_cfg.sm_mitm = mitm;
  ble_hs_cfg.sm_sc = sc;
}// setSecurityAuth

/**
 * @brief Set the authorization mode for this device.
 * @param auth_req A bitmap indicating what modes are supported.\n
 * The available bits are defined as:
 * * 0x01 BLE_SM_PAIR_AUTHREQ_BOND
 * * 0x04 BLE_SM_PAIR_AUTHREQ_MITM
 * * 0x08 BLE_SM_PAIR_AUTHREQ_SC
 * * 0x10 BLE_SM_PAIR_AUTHREQ_KEYPRESS  - not yet supported.
 */
/*STATIC*/
void Device::setSecurityAuth(uint8_t auth_req) {
  Device::setSecurityAuth((auth_req & BLE_SM_PAIR_AUTHREQ_BOND) > 0,
                          (auth_req & BLE_SM_PAIR_AUTHREQ_MITM) > 0,
                          (auth_req & BLE_SM_PAIR_AUTHREQ_SC) > 0);
}// setSecurityAuth

/**
 * @brief Set the Input/Output capabilities of this device.
 * @param iocap One of the following values:
 * * 0x00 BLE_HS_IO_DISPLAY_ONLY         DisplayOnly IO capability
 * * 0x01 BLE_HS_IO_DISPLAY_YESNO        DisplayYesNo IO capability
 * * 0x02 BLE_HS_IO_KEYBOARD_ONLY        KeyboardOnly IO capability
 * * 0x03 BLE_HS_IO_NO_INPUT_OUTPUT      NoInputNoOutput IO capability
 * * 0x04 BLE_HS_IO_KEYBOARD_DISPLAY     KeyboardDisplay Only IO capability
 */
/*STATIC*/
void Device::setSecurityIOCap(uint8_t iocap) {
  ble_hs_cfg.sm_io_cap = iocap;
}// setSecurityIOCap

/**
 * @brief If we are the initiator of the security procedure this sets the keys we will distribute.
 * @param init_key A bitmap indicating which keys to distribute during pairing.\n
 * The available bits are defined as:
 * * 0x01: BLE_SM_PAIR_KEY_DIST_ENC  - Distribute the encryption key.
 * * 0x02: BLE_SM_PAIR_KEY_DIST_ID   - Distribute the ID key (IRK).
 * * 0x04: BLE_SM_PAIR_KEY_DIST_SIGN
 * * 0x08: BLE_SM_PAIR_KEY_DIST_LINK
 */
/*STATIC*/
void Device::setSecurityInitKey(uint8_t init_key) {
  ble_hs_cfg.sm_our_key_dist = init_key;
}// setsSecurityInitKey

/**
 * @brief Set the keys we are willing to accept during pairing.
 * @param resp_key A bitmap indicating which keys to accept during pairing.
 * The available bits are defined as:
 * * 0x01: BLE_SM_PAIR_KEY_DIST_ENC  -  Accept the encryption key.
 * * 0x02: BLE_SM_PAIR_KEY_DIST_ID   -  Accept the ID key (IRK).
 * * 0x04: BLE_SM_PAIR_KEY_DIST_SIGN
 * * 0x08: BLE_SM_PAIR_KEY_DIST_LINK
 */
/*STATIC*/
void Device::setSecurityRespKey(uint8_t resp_key) {
  ble_hs_cfg.sm_their_key_dist = resp_key;
}// setsSecurityRespKey

/**
 * @brief Set the passkey the server will ask for when pairing.
 * @param [in] pin The passkey to use.
 */
/*STATIC*/
void Device::setSecurityPasskey(uint32_t pin) {
  m_passkey = pin;
}// setSecurityPasskey

/**
 * @brief Get the current passkey used for pairing.
 * @return The current passkey.
 */
/*STATIC*/
uint32_t Device::getSecurityPasskey() {
  return m_passkey;
}// getSecurityPasskey

/**
 * @brief Set the own address type.
 * @param [in] own_addr_type Own Bluetooth Device address type.\n
 * The available bits are defined as:
 * * 0x00: BLE_OWN_ADDR_PUBLIC
 * * 0x01: BLE_OWN_ADDR_RANDOM
 * * 0x02: BLE_OWN_ADDR_RPA_PUBLIC_DEFAULT
 * * 0x03: BLE_OWN_ADDR_RPA_RANDOM_DEFAULT
 * @param [in] useNRPA If true, and address type is random, uses a non-resolvable random address.
 */
/*STATIC*/
void Device::setOwnAddrType(uint8_t own_addr_type, bool useNRPA) {
  m_own_addr_type = own_addr_type;
  switch (own_addr_type) {
#ifdef CONFIG_IDF_TARGET_ESP32
  case BLE_OWN_ADDR_PUBLIC:
    ble_hs_pvcy_rpa_config(NIMBLE_HOST_DISABLE_PRIVACY);
    break;
#endif
  case BLE_OWN_ADDR_RANDOM:
    setSecurityInitKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID);
#ifdef CONFIG_IDF_TARGET_ESP32
    ble_hs_pvcy_rpa_config(useNRPA ? NIMBLE_HOST_ENABLE_NRPA : NIMBLE_HOST_ENABLE_RPA);
#endif
    break;
  case BLE_OWN_ADDR_RPA_PUBLIC_DEFAULT:
  case BLE_OWN_ADDR_RPA_RANDOM_DEFAULT:
    setSecurityInitKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID);
#ifdef CONFIG_IDF_TARGET_ESP32
    ble_hs_pvcy_rpa_config(NIMBLE_HOST_ENABLE_RPA);
#endif
    break;
  }
}// setOwnAddrType

/**
 * @brief Start the connection securing and authorization for this connection.
 * @param conn_id The connection id of the peer device.
 * @returns NimBLE stack return code, 0 = success.
 */
/* STATIC */
int Device::startSecurity(uint16_t conn_id) {
  int rc = ble_gap_security_initiate(conn_id);
  if (rc != 0) {
    NIMBLE_LOGE(LOG_TAG, "ble_gap_security_initiate: rc=%d %s", rc, Utils::returnCodeToString(rc));
  }

  return rc;
}// startSecurity

/**
 * @brief Check if the device address is on our ignore list.
 * @param [in] address The address to look for.
 * @return True if ignoring.
 */
/*STATIC*/
bool Device::isIgnored(const Address &address) {
  for (auto &it : m_ignoreList) {
    if (it.equals(address)) {
      return true;
    }
  }

  return false;
}

/**
 * @brief Add a device to the ignore list.
 * @param [in] address The address of the device we want to ignore.
 */
/*STATIC*/
void Device::addIgnored(const Address &address) {
  m_ignoreList.push_back(address);
}

/**
 * @brief Remove a device from the ignore list.
 * @param [in] address The address of the device we want to remove from the list.
 */
/*STATIC*/
void Device::removeIgnored(const Address &address) {
  for (auto it = m_ignoreList.begin(); it != m_ignoreList.end(); ++it) {
    if ((*it).equals(address)) {
      m_ignoreList.erase(it);
      return;
    }
  }
}

/**
 * @brief Set a custom callback for gap events.
 * @param [in] handler The function to call when gap events occur.
 */
/*STATIC*/
void Device::setCustomGapHandler(gap_event_handler handler) {
  m_customGapHandler = handler;
  auto returnCode = ble_gap_event_listener_register(&m_listener, m_customGapHandler, nullptr);
  if (returnCode == BLE_HS_EALREADY) {
    NIMBLE_LOGI(LOG_TAG, "Already listening to GAP events.");
  } else {
    assert(returnCode == 0);
  }
}// setCustomGapHandler

}// namespace nimble

#endif// CONFIG_BT_ENABLED
