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
#if (defined(CONFIG_BT_NIMBLE_ENABLED) && defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER) && !CONFIG_BT_NIMBLE_EXT_ADV)

#include "host/ble_gap.h"

#include "nimble/Address.hpp"
#include "nimble/UUID.hpp"

#include <vector>

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

namespace nimble {

/* COMPATIBILITY - DO NOT USE */
#define ESP_BLE_ADV_FLAG_LIMIT_DISC (0x01 << 0)
#define ESP_BLE_ADV_FLAG_GEN_DISC (0x01 << 1)
#define ESP_BLE_ADV_FLAG_BREDR_NOT_SPT (0x01 << 2)
#define ESP_BLE_ADV_FLAG_DMT_CONTROLLER_SPT (0x01 << 3)
#define ESP_BLE_ADV_FLAG_DMT_HOST_SPT (0x01 << 4)
#define ESP_BLE_ADV_FLAG_NON_LIMIT_DISC (0x00)
/* ************************* */

/**
 * @brief Advertisement data set by the programmer to be published by the %BLE server.
 */
class AdvertisementData {
  friend class Advertising;

  // Only a subset of the possible BLE architected advertisement fields are currently exposed.  Others will
  // be exposed on demand/request or as time permits.
  //
public:
  void setAppearance(uint16_t appearance);
  void setCompleteServices(const UUID &uuid);
  void setCompleteServices16(const std::vector<UUID> &v_uuid);
  void setCompleteServices32(const std::vector<UUID> &v_uuid);
  void setFlags(uint8_t);
  void setManufacturerData(const std::string &data);
  void setManufacturerData(const std::vector<uint8_t> &data);
  void setURI(const std::string &uri);
  void setName(const std::string &name);
  void setPartialServices(const UUID &uuid);
  void setPartialServices16(const std::vector<UUID> &v_uuid);
  void setPartialServices32(const std::vector<UUID> &v_uuid);
  void setServiceData(const UUID &uuid, const std::string &data);
  void setShortName(const std::string &name);
  void addData(const std::string &data);// Add data to the payload.
  void addData(char *data, size_t length);
  void addTxPower();
  void setPreferredParams(uint16_t min, uint16_t max);
  std::string getPayload();// Retrieve the current advert payload.

private:
  void setServices(bool complete, uint8_t size, std::vector<UUID> const &v_uuid);

private:
  std::string m_payload;// The payload of the advertisement.
};// NimBLEAdvertisementData

/**
 * @brief Perform and manage %BLE advertising.
 *
 * A %BLE server will want to perform advertising in order to make itself known to %BLE clients.
 */
class Advertising {
  friend class Device;
  friend class Server;

public:
  Advertising();

public:
  void addServiceUUID(const UUID &serviceUUID);
  void addServiceUUID(const char *serviceUUID);
  void removeServiceUUID(const UUID &serviceUUID);
  bool start(uint32_t duration = 0, void (*advCompleteCB)(Advertising *pAdv) = nullptr, Address *dirAddr = nullptr);
  bool stop();
  void setAppearance(uint16_t appearance);
  void setName(const std::string &name);
  void setManufacturerData(const std::string &data);
  void setManufacturerData(const std::vector<uint8_t> &data);
  void setURI(const std::string &uri);
  void setServiceData(const UUID &uuid, const std::string &data);
  void setAdvertisementType(uint8_t adv_type);
  void setMaxInterval(uint16_t maxinterval);
  void setMinInterval(uint16_t mininterval);
  void setAdvertisementData(AdvertisementData &advertisementData);
  void setScanFilter(bool scanRequestWhitelistOnly, bool connectWhitelistOnly);
  void setScanResponseData(AdvertisementData &advertisementData);
  void setScanResponse(bool);
  void setMinPreferred(uint16_t);
  void setMaxPreferred(uint16_t);
  void addTxPower();
  void reset();
  void advCompleteCB();
  bool isAdvertising();

private:
  void onHostSync();
  static int handleGapEvent(struct ble_gap_event *event, void *arg);

  ble_hs_adv_fields m_advData;
  ble_hs_adv_fields m_scanData;
  ble_gap_adv_params m_advParams;
  std::vector<UUID> m_serviceUUIDs;
  bool m_customAdvData;
  bool m_customScanResponseData;
  bool m_scanResp;
  bool m_advDataSet;
  void (*m_advCompCB)(Advertising *pAdv);
  uint8_t m_slaveItvl[4];
  uint32_t m_duration;
  std::vector<uint8_t> m_svcData16;
  std::vector<uint8_t> m_svcData32;
  std::vector<uint8_t> m_svcData128;
  std::vector<uint8_t> m_name;
  std::vector<uint8_t> m_mfgData;
  std::vector<uint8_t> m_uri;
};

}// namespace nimble

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_BROADCASTER  && !CONFIG_BT_NIMBLE_EXT_ADV */
