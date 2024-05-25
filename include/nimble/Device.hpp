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
#if defined(CONFIG_BT_NIMBLE_ENABLED)

#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
#include "nimble/Scan.hpp"
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
#if CONFIG_BT_NIMBLE_EXT_ADV
#include "nimble/NimBLEExtAdvertising.hpp"
#else
#include "nimble/Advertising.hpp"
#endif
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)
#include "nimble/Client.hpp"
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
#include "nimble/Server.hpp"
#endif

#include "nimble/Address.hpp"
#include "nimble/Utils.hpp"

#include "esp_bt.h"

#include <list>
#include <map>
#include <string>

typedef int (*gap_event_handler)(ble_gap_event *event, void *arg);

extern "C" void ble_store_config_init(void);

namespace nimble {

/**
 * @brief A model of a %BLE Device from which all the BLE roles are created.
 */
class Device {
public:
  static void init(const std::string &deviceName);
  static void deinit(bool clearAll = false);
  static void setDeviceName(const std::string &deviceName);
  static bool getInitialized();
  static Address getAddress();
  static std::string toString();
  static bool whiteListAdd(const Address &address);
  static bool whiteListRemove(const Address &address);
  static bool onWhiteList(const Address &address);
  static size_t getWhiteListCount();
  static Address getWhiteListAddress(size_t index);

#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
  static Scan *getScan();
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
  static Server *createServer();
  static Server *getServer();
#endif

#ifdef ESP_PLATFORM
  static void setPower(esp_power_level_t powerLevel, esp_ble_power_type_t powerType = ESP_BLE_PWR_TYPE_DEFAULT);
  static int getPower(esp_ble_power_type_t powerType = ESP_BLE_PWR_TYPE_DEFAULT);
  static void setOwnAddrType(uint8_t own_addr_type, bool useNRPA = false);
  static void setScanDuplicateCacheSize(uint16_t cacheSize);
  static void setScanFilterMode(uint8_t type);
#else
  static void setPower(int dbm);
  static int getPower();
#endif

  static void setCustomGapHandler(gap_event_handler handler);
  static void setSecurityAuth(bool bonding, bool mitm, bool sc);
  static void setSecurityAuth(uint8_t auth_req);
  static void setSecurityIOCap(uint8_t iocap);
  static void setSecurityInitKey(uint8_t init_key);
  static void setSecurityRespKey(uint8_t init_key);
  static void setSecurityPasskey(uint32_t pin);
  static uint32_t getSecurityPasskey();
  static int startSecurity(uint16_t conn_id);
  static int setMTU(uint16_t mtu);
  static uint16_t getMTU();
  static bool isIgnored(const Address &address);
  static void addIgnored(const Address &address);
  static void removeIgnored(const Address &address);

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
#if CONFIG_BT_NIMBLE_EXT_ADV
  static NimBLEExtAdvertising *getAdvertising();
  static bool startAdvertising(uint8_t inst_id,
                               int duration = 0,
                               int max_events = 0);
  static bool stopAdvertising(uint8_t inst_id);
  static bool stopAdvertising();
#endif
#if !CONFIG_BT_NIMBLE_EXT_ADV || defined(_DOXYGEN_)
  static Advertising *getAdvertising();
  static bool startAdvertising(uint32_t duration = 0);
  static bool stopAdvertising();
#endif
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)
  static Client *createClient(Address peerAddress = Address(""));
  static bool deleteClient(Client *pClient);
  static Client *getClientByID(uint16_t conn_id);
  static Client *getClientByPeerAddress(const Address &peer_addr);
  static Client *getDisconnectedClient();
  static size_t getClientListSize();
  static std::list<Client *> *getClientList();
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL) || defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
  static bool deleteBond(const Address &address);
  static int getNumBonds();
  static bool isBonded(const Address &address);
  static void deleteAllBonds();
  static Address getBondedAddress(int index);
#endif

private:
#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)
  friend class Client;
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
  friend class Scan;
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
  friend class Server;
  friend class Characteristic;
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
  friend class Advertising;
#if CONFIG_BT_NIMBLE_EXT_ADV
  friend class NimBLEExtAdvertising;
  friend class NimBLEExtAdvertisement;
#endif
#endif

  static void onReset(int reason);
  static void onSync(void);
  static void host_task(void *param);
  static bool m_synced;

#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
  static Scan *m_pScan;
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
  static Server *m_pServer;
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
#if CONFIG_BT_NIMBLE_EXT_ADV
  static NimBLEExtAdvertising *m_bleAdvertising;
#else
  static Advertising *m_bleAdvertising;
#endif
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)
  static std::list<Client *> m_cList;
#endif
  static std::list<Address> m_ignoreList;
  static uint32_t m_passkey;
  static ble_gap_event_listener m_listener;
  static gap_event_handler m_customGapHandler;
  static uint8_t m_own_addr_type;
#ifdef CONFIG_BTDM_BLE_SCAN_DUPL
  static uint16_t m_scanDuplicateSize;
  static uint8_t m_scanFilterMode;
#endif
  static std::vector<Address> m_whiteList;
};

}

#endif// CONFIG_BT_ENABLED
