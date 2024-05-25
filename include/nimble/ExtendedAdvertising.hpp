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
#if defined(CONFIG_BT_NIMBLE_ENABLED) && \
    defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER) && \
    CONFIG_BT_NIMBLE_EXT_ADV

#    include "host/ble_gap.h"

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

#include "nimble/NimBLEAddress.hpp"
#include "nimble/NimBLEUUID.hpp"

#include <vector>

namespace nimble {

class ExtendedAdvertisingCallbacks;

/**
 * @brief Extended advertisement data
 */
class ExtendedAdvertisement {
public:
  ExtendedAdvertisement(uint8_t priPhy = BLE_HCI_LE_PHY_1M, uint8_t secPhy = BLE_HCI_LE_PHY_1M);
  void setAppearance(uint16_t appearance);
  void setCompleteServices(const NimBLEUUID &uuid);
  void setCompleteServices16(const std::vector<NimBLEUUID> &v_uuid);
  void setCompleteServices32(const std::vector<NimBLEUUID> &v_uuid);
  void setFlags(uint8_t flag);
  void setManufacturerData(const std::string &data);
  void setURI(const std::string &uri);
  void setName(const std::string &name);
  void setPartialServices(const NimBLEUUID &uuid);
  void setPartialServices16(const std::vector<NimBLEUUID> &v_uuid);
  void setPartialServices32(const std::vector<NimBLEUUID> &v_uuid);
  void setServiceData(const NimBLEUUID &uuid, const std::string &data);
  void setShortName(const std::string &name);
  void setData(const uint8_t *data, size_t length);
  void addData(const std::string &data);
  void addData(const uint8_t *data, size_t length);
  void addTxPower();
  void setPreferredParams(uint16_t min, uint16_t max);
  void setLegacyAdvertising(bool val);
  void setConnectable(bool val);
  void setScannable(bool val);
  void setMinInterval(uint32_t mininterval);
  void setMaxInterval(uint32_t maxinterval);
  void setPrimaryPhy(uint8_t phy);
  void setSecondaryPhy(uint8_t phy);
  void setScanFilter(bool scanRequestWhitelistOnly, bool connectWhitelistOnly);
  void setDirectedPeer(const NimBLEAddress &addr);
  void setDirected(bool val, bool high_duty = true);
  void setAnonymous(bool val);
  void setPrimaryChannels(bool ch37, bool ch38, bool ch39);
  void setTxPower(int8_t dbm);
  void setAddress(const NimBLEAddress &addr);
  void enableScanRequestCallback(bool enable);
  void clearData();
  size_t getDataSize();

private:
  friend class NimBLEExtAdvertising;

  void setServices(const bool complete, const uint8_t size,
                   const std::vector<NimBLEUUID> &v_uuid);

  std::vector<uint8_t> m_payload;
  ble_gap_ext_adv_params m_params;
  NimBLEAddress m_advAddress;
};// NimBLEExtAdvertisement

/**
 * @brief Extended advertising class.
 */
class ExtendedAdvertising {
public:
  /**
     * @brief Construct an extended advertising object.
     */
  ExtendedAdvertising() : m_advStatus(CONFIG_BT_NIMBLE_MAX_EXT_ADV_INSTANCES + 1, false) {}
  ~ExtendedAdvertising();
  bool start(uint8_t inst_id, int duration = 0, int max_events = 0);
  bool setInstanceData(uint8_t inst_id, NimBLEExtAdvertisement &adv);
  bool setScanResponseData(uint8_t inst_id, NimBLEExtAdvertisement &data);
  bool removeInstance(uint8_t inst_id);
  bool removeAll();
  bool stop(uint8_t inst_id);
  bool stop();
  bool isActive(uint8_t inst_id);
  bool isAdvertising();
  void setCallbacks(ExtendedAdvertisingCallbacks *callbacks, bool deleteCallbacks = true);

private:
  friend class NimBLEDevice;
  friend class NimBLEServer;

  void onHostSync();
  static int handleGapEvent(struct ble_gap_event *event, void *arg);

  bool m_scanResp;
  bool m_deleteCallbacks;
  ExtendedAdvertisingCallbacks *m_pCallbacks;
  ble_gap_ext_adv_params m_advParams;
  std::vector<bool> m_advStatus;
};

/**
 * @brief Callbacks associated with NimBLEExtAdvertising class.
 */
class ExtendedAdvertisingCallbacks {
public:
  virtual ~ExtendedAdvertisingCallbacks() = default;

  /**
     * @brief Handle an advertising stop event.
     * @param [in] pAdv A convenience pointer to the extended advertising interface.
     * @param [in] reason The reason code for stopping the advertising.
     * @param [in] inst_id The instance ID of the advertisement that was stopped.
     */
  virtual void onStopped(NimBLEExtAdvertising *pAdv, int reason, uint8_t inst_id);

  /**
     * @brief Handle a scan response request.
     * This is called when a scanning device requests a scan response.
     * @param [in] pAdv A convenience pointer to the extended advertising interface.
     * @param [in] inst_id The instance ID of the advertisement that the scan response request was made.
     * @param [in] addr The address of the device making the request.
     */
  virtual void onScanRequest(NimBLEExtAdvertising *pAdv, uint8_t inst_id, NimBLEAddress addr);
};// NimBLEExtAdvertisingCallbacks

}

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_BROADCASTER && CONFIG_BT_NIMBLE_EXT_ADV */
