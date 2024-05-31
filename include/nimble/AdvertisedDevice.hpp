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
#if defined(CONFIG_BT_NIMBLE_ENABLED) && defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)

#include "nimble/Address.hpp"
#include "nimble/Scan.hpp"
#include "nimble/UUID.hpp"

#include "host/ble_hs_adv.h"

#include <ctime>
#include <map>
#include <vector>

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

namespace nimble {

class Scan;
/**
 * @brief A representation of a %BLE advertised device found by a scan.
 *
 * When we perform a %BLE scan, the result will be a set of devices that are advertising.  This
 * class provides a model of a detected device.
 */
class AdvertisedDevice {
public:
  AdvertisedDevice();

  Address getAddress();
  uint8_t getAdvType();
  uint8_t getAdvFlags();
  uint16_t getAppearance();
  uint16_t getAdvInterval();
  uint16_t getMinInterval();
  uint16_t getMaxInterval();
  uint8_t getManufacturerDataCount();
  std::string getManufacturerData(uint8_t index = 0);
  std::string getURI();
  std::string getPayloadByType(uint16_t type);

  /**
     * @brief A template to convert the service data to <type\>.
     * @tparam T The type to convert the data to.
     * @param [in] skipSizeCheck If true it will skip checking if the data size is less than <tt>sizeof(<type\>)</tt>.
     * @return The data converted to <type\> or NULL if skipSizeCheck is false and the data is
     * less than <tt>sizeof(<type\>)</tt>.
     * @details <b>Use:</b> <tt>getManufacturerData<type>(skipSizeCheck);</tt>
     */
  template<typename T>
  T getManufacturerData(bool skipSizeCheck = false) {
    std::string data = getManufacturerData();
    if (!skipSizeCheck && data.size() < sizeof(T))
      return T();
    const char *pData = data.data();
    return *((T *) pData);
  }

  std::string getName();
  int getRSSI();
  Scan *getScan();
  uint8_t getServiceDataCount();
  std::string getServiceData(uint8_t index = 0);
  std::string getServiceData(const UUID &uuid);

  /**
     * @brief A template to convert the service data to <tt><type\></tt>.
     * @tparam T The type to convert the data to.
     * @param [in] index The vector index of the service data requested.
     * @param [in] skipSizeCheck If true it will skip checking if the data size is less than <tt>sizeof(<type\>)</tt>.
     * @return The data converted to <type\> or NULL if skipSizeCheck is false and the data is
     * less than <tt>sizeof(<type\>)</tt>.
     * @details <b>Use:</b> <tt>getServiceData<type>(skipSizeCheck);</tt>
     */
  template<typename T>
  T getServiceData(uint8_t index = 0, bool skipSizeCheck = false) {
    std::string data = getServiceData(index);
    if (!skipSizeCheck && data.size() < sizeof(T))
      return T();
    const char *pData = data.data();
    return *((T *) pData);
  }

  /**
     * @brief A template to convert the service data to <tt><type\></tt>.
     * @tparam T The type to convert the data to.
     * @param [in] uuid The uuid of the service data requested.
     * @param [in] skipSizeCheck If true it will skip checking if the data size is less than <tt>sizeof(<type\>)</tt>.
     * @return The data converted to <type\> or NULL if skipSizeCheck is false and the data is
     * less than <tt>sizeof(<type\>)</tt>.
     * @details <b>Use:</b> <tt>getServiceData<type>(skipSizeCheck);</tt>
     */
  template<typename T>
  T getServiceData(const UUID &uuid, bool skipSizeCheck = false) {
    std::string data = getServiceData(uuid);
    if (!skipSizeCheck && data.size() < sizeof(T))
      return T();
    const char *pData = data.data();
    return *((T *) pData);
  }

  UUID getServiceDataUUID(uint8_t index = 0);
  UUID getServiceUUID(uint8_t index = 0);
  uint8_t getServiceUUIDCount();
  Address getTargetAddress(uint8_t index = 0);
  uint8_t getTargetAddressCount();
  int8_t getTXPower();
  uint8_t *getPayload();
  uint8_t getAdvLength();
  size_t getPayloadLength();
  uint8_t getAddressType();
  time_t getTimestamp();
  bool isAdvertisingService(const UUID &uuid);
  bool haveAppearance();
  bool haveManufacturerData();
  bool haveName();
  bool haveRSSI();
  bool haveServiceData();
  bool haveServiceUUID();
  bool haveTXPower();
  bool haveConnParams();
  bool haveAdvInterval();
  bool haveTargetAddress();
  bool haveURI();
  bool haveType(uint16_t type);
  std::string toString();
  bool isConnectable();
  bool isLegacyAdvertisement();
#if CONFIG_BT_NIMBLE_EXT_ADV
  uint8_t getSetId();
  uint8_t getPrimaryPhy();
  uint8_t getSecondaryPhy();
  uint16_t getPeriodicInterval();
#endif

private:
  friend class Scan;

  void setAddress(Address address);
  void setAdvType(uint8_t advType, bool isLegacyAdv);
  void setPayload(const uint8_t *payload, uint8_t length, bool append);
  void setRSSI(int rssi);
#if CONFIG_BT_NIMBLE_EXT_ADV
  void setSetId(uint8_t sid) { m_sid = sid; }
  void setPrimaryPhy(uint8_t phy) { m_primPhy = phy; }
  void setSecondaryPhy(uint8_t phy) { m_secPhy = phy; }
  void setPeriodicInterval(uint16_t itvl) { m_periodicItvl = itvl; }
#endif
  uint8_t findAdvField(uint8_t type, uint8_t index = 0, size_t *data_loc = nullptr);
  size_t findServiceData(uint8_t index, uint8_t *bytes);

  Address m_address = Address("");
  uint8_t m_advType;
  int m_rssi;
  time_t m_timestamp;
  uint8_t m_callbackSent;
  uint8_t m_advLength;
#if CONFIG_BT_NIMBLE_EXT_ADV
  bool m_isLegacyAdv;
  uint8_t m_sid;
  uint8_t m_primPhy;
  uint8_t m_secPhy;
  uint16_t m_periodicItvl;
#endif

  std::vector<uint8_t> m_payload;
};

}// namespace nimble

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_OBSERVER */
