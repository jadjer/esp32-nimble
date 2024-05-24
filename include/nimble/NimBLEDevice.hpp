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
#include "nimble/NimBLEScan.hpp"
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
#  if CONFIG_BT_NIMBLE_EXT_ADV
#    include "nimble/NimBLEExtAdvertising.hpp"
#  else
#    include "nimble/NimBLEAdvertising.hpp"
#  endif
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)
#include "nimble/NimBLEClient.hpp"
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
#include "nimble/NimBLEServer.hpp"
#endif

#include "nimble/NimBLEUtils.hpp"
#include "nimble/NimBLEAddress.hpp"

#  include "esp_bt.h"

#include <map>
#include <string>
#include <list>

#define BLEDevice                       NimBLEDevice
#define BLEClient                       NimBLEClient
#define BLERemoteService                NimBLERemoteService
#define BLERemoteCharacteristic         NimBLERemoteCharacteristic
#define BLERemoteDescriptor             NimBLERemoteDescriptor
#define BLEAdvertisedDevice             NimBLEAdvertisedDevice
#define BLEScan                         NimBLEScan
#define BLEUUID                         NimBLEUUID
#define BLESecurity                     NimBLESecurity
#define BLESecurityCallbacks            NimBLESecurityCallbacks
#define BLEAddress                      NimBLEAddress
#define BLEUtils                        NimBLEUtils
#define BLEClientCallbacks              NimBLEClientCallbacks
#define BLEAdvertisedDeviceCallbacks    NimBLEScanCallbacks
#define BLEScanResults                  NimBLEScanResults
#define BLEServer                       NimBLEServer
#define BLEService                      NimBLEService
#define BLECharacteristic               NimBLECharacteristic
#define BLEAdvertising                  NimBLEAdvertising
#define BLEServerCallbacks              NimBLEServerCallbacks
#define BLECharacteristicCallbacks      NimBLECharacteristicCallbacks
#define BLEAdvertisementData            NimBLEAdvertisementData
#define BLEDescriptor                   NimBLEDescriptor
#define BLE2902                         NimBLE2902
#define BLE2904                         NimBLE2904
#define BLEDescriptorCallbacks          NimBLEDescriptorCallbacks
#define BLEBeacon                       NimBLEBeacon
#define BLEEddystoneTLM                 NimBLEEddystoneTLM
#define BLEEddystoneURL                 NimBLEEddystoneURL
#define BLEConnInfo                     NimBLEConnInfo

typedef int (*gap_event_handler)(ble_gap_event *event, void *arg);

extern "C" void ble_store_config_init(void);

/**
 * @brief A model of a %BLE Device from which all the BLE roles are created.
 */
class NimBLEDevice {
public:
    static void             init(const std::string &deviceName);
    static void             deinit(bool clearAll = false);
    static void             setDeviceName(const std::string &deviceName);
    static bool             getInitialized();
    static NimBLEAddress    getAddress();
    static std::string      toString();
    static bool             whiteListAdd(const NimBLEAddress & address);
    static bool             whiteListRemove(const NimBLEAddress & address);
    static bool             onWhiteList(const NimBLEAddress & address);
    static size_t           getWhiteListCount();
    static NimBLEAddress    getWhiteListAddress(size_t index);

#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
    static NimBLEScan*      getScan();
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
    static NimBLEServer*    createServer();
    static NimBLEServer*    getServer();
#endif

#ifdef ESP_PLATFORM
    static void             setPower(esp_power_level_t powerLevel, esp_ble_power_type_t powerType=ESP_BLE_PWR_TYPE_DEFAULT);
    static int              getPower(esp_ble_power_type_t powerType=ESP_BLE_PWR_TYPE_DEFAULT);
    static void             setOwnAddrType(uint8_t own_addr_type, bool useNRPA=false);
    static void             setScanDuplicateCacheSize(uint16_t cacheSize);
    static void             setScanFilterMode(uint8_t type);
#else
    static void             setPower(int dbm);
    static int              getPower();
#endif

    static void             setCustomGapHandler(gap_event_handler handler);
    static void             setSecurityAuth(bool bonding, bool mitm, bool sc);
    static void             setSecurityAuth(uint8_t auth_req);
    static void             setSecurityIOCap(uint8_t iocap);
    static void             setSecurityInitKey(uint8_t init_key);
    static void             setSecurityRespKey(uint8_t init_key);
    static void             setSecurityPasskey(uint32_t pin);
    static uint32_t         getSecurityPasskey();
    static int              startSecurity(uint16_t conn_id);
    static int              setMTU(uint16_t mtu);
    static uint16_t         getMTU();
    static bool             isIgnored(const NimBLEAddress &address);
    static void             addIgnored(const NimBLEAddress &address);
    static void             removeIgnored(const NimBLEAddress &address);

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
#  if CONFIG_BT_NIMBLE_EXT_ADV
    static NimBLEExtAdvertising* getAdvertising();
    static bool                  startAdvertising(uint8_t inst_id,
                                                  int duration = 0,
                                                  int max_events = 0);
    static bool                  stopAdvertising(uint8_t inst_id);
    static bool                  stopAdvertising();
#  endif
#  if !CONFIG_BT_NIMBLE_EXT_ADV || defined(_DOXYGEN_)
    static NimBLEAdvertising*    getAdvertising();
    static bool                  startAdvertising(uint32_t duration = 0);
    static bool                  stopAdvertising();
#  endif
#endif

#if defined( CONFIG_BT_NIMBLE_ROLE_CENTRAL)
    static NimBLEClient*    createClient(NimBLEAddress peerAddress = NimBLEAddress(""));
    static bool             deleteClient(NimBLEClient* pClient);
    static NimBLEClient*    getClientByID(uint16_t conn_id);
    static NimBLEClient*    getClientByPeerAddress(const NimBLEAddress &peer_addr);
    static NimBLEClient*    getDisconnectedClient();
    static size_t           getClientListSize();
    static std::list<NimBLEClient*>* getClientList();
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL) || defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
    static bool             deleteBond(const NimBLEAddress &address);
    static int              getNumBonds();
    static bool             isBonded(const NimBLEAddress &address);
    static void             deleteAllBonds();
    static NimBLEAddress    getBondedAddress(int index);
#endif

private:
#if defined( CONFIG_BT_NIMBLE_ROLE_CENTRAL)
    friend class NimBLEClient;
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
    friend class NimBLEScan;
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
    friend class NimBLEServer;
    friend class NimBLECharacteristic;
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
    friend class NimBLEAdvertising;
#  if CONFIG_BT_NIMBLE_EXT_ADV
    friend class NimBLEExtAdvertising;
    friend class NimBLEExtAdvertisement;
#  endif
#endif

    static void        onReset(int reason);
    static void        onSync(void);
    static void        host_task(void *param);
    static bool        m_synced;

#if defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)
    static NimBLEScan*                m_pScan;
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
    static NimBLEServer*              m_pServer;
#endif

#if defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)
#  if CONFIG_BT_NIMBLE_EXT_ADV
    static NimBLEExtAdvertising*      m_bleAdvertising;
#  else
    static NimBLEAdvertising*         m_bleAdvertising;
#  endif
#endif

#if defined( CONFIG_BT_NIMBLE_ROLE_CENTRAL)
    static std::list <NimBLEClient*>  m_cList;
#endif
    static std::list <NimBLEAddress>  m_ignoreList;
    static uint32_t                   m_passkey;
    static ble_gap_event_listener     m_listener;
    static gap_event_handler          m_customGapHandler;
    static uint8_t                    m_own_addr_type;
#ifdef ESP_PLATFORM
#  ifdef CONFIG_BTDM_BLE_SCAN_DUPL
    static uint16_t                   m_scanDuplicateSize;
    static uint8_t                    m_scanFilterMode;
#  endif
#endif
    static std::vector<NimBLEAddress> m_whiteList;
};


#endif // CONFIG_BT_ENABLED
