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

#include <list>
#include <map>
#include <string>

#include <esp_bt.h>

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
#include "nimble/Scan.hpp"
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
#include "nimble/Advertising.hpp"
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)
#include "nimble/Client.hpp"
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
#include "nimble/Server.hpp"
#endif

#include "nimble/Address.hpp"
#include "nimble/Utils.hpp"

typedef int (*gap_event_handler)(ble_gap_event *event, void *arg);

extern "C" void ble_store_config_init(void);



namespace nimble {

/**
 * @brief A model of a %BLE Device from which all the BLE roles are created.
 */
class Device {
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
#endif

public:
  static void init(std::string const &deviceName);
  static void deinit(bool clearAll = false);
  static void setDeviceName(std::string const &deviceName);
  static bool isInitialized();
  static Address getAddress();
  static std::string toString();
  static bool whiteListAdd(const Address &address);
  static bool whiteListRemove(const Address &address);
  static bool onWhiteList(const Address &address);
  static size_t getWhiteListCount();
  static Address getWhiteListAddress(size_t index);

#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
public:
  static Scan *getScan();
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
public:
  static Server *createServer();
  static Server *getServer();
#endif

public:
  static void setPower(esp_power_level_t powerLevel, esp_ble_power_type_t powerType = ESP_BLE_PWR_TYPE_DEFAULT);
  static int8_t getPower(esp_ble_power_type_t powerType = ESP_BLE_PWR_TYPE_DEFAULT);
  static void setOwnAddrType(uint8_t own_addr_type, bool useNRPA = false);
  static void setScanDuplicateCacheSize(uint16_t cacheSize);
  static void setScanFilterMode(uint8_t type);

public:
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
public:
  static Advertising *getAdvertising();
  static bool startAdvertising(uint32_t duration = 0);
  static bool stopAdvertising();
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)
public:
  static Client *createClient(Address peerAddress = Address(""));
  static bool deleteClient(Client *pClient);
  static Client *getClientByID(uint16_t conn_id);
  static Client *getClientByPeerAddress(const Address &peer_addr);
  static Client *getDisconnectedClient();
  static size_t getClientListSize();
  static std::list<Client *> *getClientList();
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL) || defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
public:
  static bool deleteBond(const Address &address);
  static int getNumBonds();
  static bool isBonded(const Address &address);
  static void deleteAllBonds();
  static Address getBondedAddress(int index);
#endif

private:
  static void onReset(int reason);
  static void onSync();
  static void hostTask(void *param);

private:
  static bool m_isInitialized;
  static bool m_isSynced;
  static std::string m_deviceName;

#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
  static Scan *m_pScan;
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
  static Server *m_pServer;
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
  static Advertising *m_bleAdvertising;
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

}// namespace nimble

#endif// CONFIG_BT_ENABLED
