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

#include "nimble/Beacon.hpp"
#include "nimble/Log.hpp"
#include <algorithm>
#include <cstring>

#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00)>>8) + (((x)&0xFF)<<8))

static const char* LOG_TAG = "NimBLEBeacon";


namespace nimble {

/**
 * @brief Construct a default beacon object.
 */
Beacon::Beacon() {
  m_beaconData.manufacturerId = 0x4c00;
  m_beaconData.subType = 0x02;
  m_beaconData.subTypeLength = 0x15;
  m_beaconData.major = 0;
  m_beaconData.minor = 0;
  m_beaconData.signalPower = 0;
  memset(m_beaconData.proximityUUID, 0, sizeof(m_beaconData.proximityUUID));
}// NimBLEBeacon

/**
 * @brief Retrieve the data that is being advertised.
 * @return The advertised data.
 */
std::string Beacon::getData() {
  return std::string((char *) &m_beaconData, sizeof(m_beaconData));
}// getData

/**
 * @brief Get the major value being advertised.
 * @return The major value advertised.
 */
uint16_t Beacon::getMajor() {
  return m_beaconData.major;
}

/**
 * @brief Get the manufacturer ID being advertised.
 * @return The manufacturer ID value advertised.
 */
uint16_t Beacon::getManufacturerId() {
  return m_beaconData.manufacturerId;
}

/**
 * @brief Get the minor value being advertised.
 * @return minor value advertised.
 */
uint16_t Beacon::getMinor() {
  return m_beaconData.minor;
}

/**
 * @brief Get the proximity UUID being advertised.
 * @return The UUID advertised.
 */
UUID Beacon::getProximityUUID() {
  return UUID(m_beaconData.proximityUUID, 16, true);
}

/**
 * @brief Get the signal power being advertised.
 * @return signal power level advertised.
 */
int8_t Beacon::getSignalPower() {
  return m_beaconData.signalPower;
}

/**
 * @brief Set the raw data for the beacon record.
 * @param [in] data The raw beacon data.
 */
void Beacon::setData(const std::string &data) {
  if (data.length() != sizeof(m_beaconData)) {
    NIMBLE_LOGE(LOG_TAG, "Unable to set the data ... length passed in was %d and expected %d",
                data.length(), sizeof(m_beaconData));
    return;
  }
  memcpy(&m_beaconData, data.data(), sizeof(m_beaconData));
}// setData

/**
 * @brief Set the major value.
 * @param [in] major The major value.
 */
void Beacon::setMajor(uint16_t major) {
  m_beaconData.major = ENDIAN_CHANGE_U16(major);
}// setMajor

/**
 * @brief Set the manufacturer ID.
 * @param [in] manufacturerId The manufacturer ID value.
 */
void Beacon::setManufacturerId(uint16_t manufacturerId) {
  m_beaconData.manufacturerId = ENDIAN_CHANGE_U16(manufacturerId);
}// setManufacturerId

/**
 * @brief Set the minor value.
 * @param [in] minor The minor value.
 */
void Beacon::setMinor(uint16_t minor) {
  m_beaconData.minor = ENDIAN_CHANGE_U16(minor);
}// setMinor

/**
 * @brief Set the proximity UUID.
 * @param [in] uuid The proximity UUID.
 */
void Beacon::setProximityUUID(const UUID &uuid) {
  UUID temp_uuid = uuid;
  temp_uuid.to128();
  std::reverse_copy(temp_uuid.getNative()->u128.value,
                    temp_uuid.getNative()->u128.value + 16,
                    m_beaconData.proximityUUID);
}// setProximityUUID

/**
 * @brief Set the signal power.
 * @param [in] signalPower The signal power value.
 */
void Beacon::setSignalPower(int8_t signalPower) {
  m_beaconData.signalPower = signalPower;
}// setSignalPower

}

#endif
