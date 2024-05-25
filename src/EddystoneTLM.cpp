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

#include "nimble/EddystoneTLM.hpp"
#include "nimble/Log.hpp"

#include <cstdio>
#include <cstring>

#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00)>>8) + (((x)&0xFF)<<8))
#define ENDIAN_CHANGE_U32(x) ((((x)&0xFF000000)>>24) + (((x)&0x00FF0000)>>8)) + ((((x)&0xFF00)<<8) + (((x)&0xFF)<<24))

static const char LOG_TAG[] = "NimBLEEddystoneTLM";

namespace nimble {

/**
 * @brief Construct a default EddystoneTLM beacon object.
 */
EddystoneTLM::EddystoneTLM() {
  beaconUUID = 0xFEAA;
  m_eddystoneData.frameType = EDDYSTONE_TLM_FRAME_TYPE;
  m_eddystoneData.version = 0;
  m_eddystoneData.volt = 3300;                            // 3300mV = 3.3V
  m_eddystoneData.temp = (uint16_t) ((float) 23.00 * 256);// 8.8 fixed format
  m_eddystoneData.advCount = 0;
  m_eddystoneData.tmil = 0;
}// NimBLEEddystoneTLM

/**
 * @brief Retrieve the data that is being advertised.
 * @return The advertised data.
 */
std::string EddystoneTLM::getData() {
  return std::string((char *) &m_eddystoneData, sizeof(m_eddystoneData));
}// getData

/**
 * @brief Get the UUID being advertised.
 * @return The UUID advertised.
 */
UUID EddystoneTLM::getUUID() {
  return UUID(beaconUUID);
}// getUUID

/**
 * @brief Get the version being advertised.
 * @return The version number.
 */
uint8_t EddystoneTLM::getVersion() {
  return m_eddystoneData.version;
}// getVersion

/**
 * @brief Get the battery voltage.
 * @return The battery voltage.
 */
uint16_t EddystoneTLM::getVolt() {
  return ENDIAN_CHANGE_U16(m_eddystoneData.volt);
}// getVolt

/**
 * @brief Get the temperature being advertised.
 * @return The temperature value.
 */
float EddystoneTLM::getTemp() {
  return ENDIAN_CHANGE_U16(m_eddystoneData.temp) / 256.0f;
}// getTemp

/**
 * @brief Get the count of advertisements sent.
 * @return The number of advertisements.
 */
uint32_t EddystoneTLM::getCount() {
  return ENDIAN_CHANGE_U32(m_eddystoneData.advCount);
}// getCount

/**
 * @brief Get the advertisement time.
 * @return The advertisement time.
 */
uint32_t EddystoneTLM::getTime() {
  return (ENDIAN_CHANGE_U32(m_eddystoneData.tmil)) / 10;
}// getTime

/**
 * @brief Get a string representation of the beacon.
 * @return The string representation.
 */
std::string EddystoneTLM::toString() {
  std::string out = "";
  uint32_t rawsec = ENDIAN_CHANGE_U32(m_eddystoneData.tmil);
  char val[12];

  out += "Version ";// + std::string(m_eddystoneData.version);
  snprintf(val, sizeof(val), "%d", m_eddystoneData.version);
  out += val;
  out += "\n";
  out += "Battery Voltage ";// + ENDIAN_CHANGE_U16(m_eddystoneData.volt);
  snprintf(val, sizeof(val), "%d", ENDIAN_CHANGE_U16(m_eddystoneData.volt));
  out += val;
  out += " mV\n";

  out += "Temperature ";
  snprintf(val, sizeof(val), "%.2f", ENDIAN_CHANGE_U16(m_eddystoneData.temp) / 256.0f);
  out += val;
  out += " C\n";

  out += "Adv. Count ";
  snprintf(val, sizeof(val), "%" PRIu32, ENDIAN_CHANGE_U32(m_eddystoneData.advCount));
  out += val;
  out += "\n";

  out += "Time in seconds ";
  snprintf(val, sizeof(val), "%" PRIu32, rawsec / 10);
  out += val;
  out += "\n";

  out += "Time ";

  snprintf(val, sizeof(val), "%04" PRIu32, rawsec / 864000);
  out += val;
  out += ".";

  snprintf(val, sizeof(val), "%02" PRIu32, (rawsec / 36000) % 24);
  out += val;
  out += ":";

  snprintf(val, sizeof(val), "%02" PRIu32, (rawsec / 600) % 60);
  out += val;
  out += ":";

  snprintf(val, sizeof(val), "%02" PRIu32, (rawsec / 10) % 60);
  out += val;
  out += "\n";

  return out;
}// toString

/**
 * @brief Set the raw data for the beacon advertisement.
 * @param [in] data The raw data to advertise.
 */
void EddystoneTLM::setData(const std::string &data) {
  if (data.length() != sizeof(m_eddystoneData)) {
    NIMBLE_LOGE(LOG_TAG, "Unable to set the data ... length passed in was %d and expected %d",
                data.length(), sizeof(m_eddystoneData));
    return;
  }
  memcpy(&m_eddystoneData, data.data(), data.length());
}// setData

/**
 * @brief Set the UUID to advertise.
 * @param [in] l_uuid The UUID.
 */
void EddystoneTLM::setUUID(const UUID &l_uuid) {
  beaconUUID = l_uuid.getNative()->u16.value;
}// setUUID

/**
 * @brief Set the version to advertise.
 * @param [in] version The version number.
 */
void EddystoneTLM::setVersion(uint8_t version) {
  m_eddystoneData.version = version;
}// setVersion

/**
 * @brief Set the battery voltage to advertise.
 * @param [in] volt The voltage in millivolts.
 */
void EddystoneTLM::setVolt(uint16_t volt) {
  m_eddystoneData.volt = volt;
}// setVolt

/**
 * @brief Set the temperature to advertise.
 * @param [in] temp The temperature value.
 */
void EddystoneTLM::setTemp(float temp) {
  m_eddystoneData.temp = (uint16_t) temp;
}// setTemp

/**
 * @brief Set the advertisement count.
 * @param [in] advCount The advertisement number.
 */
void EddystoneTLM::setCount(uint32_t advCount) {
  m_eddystoneData.advCount = advCount;
}// setCount

/**
 * @brief Set the advertisement time.
 * @param [in] tmil The advertisement time in milliseconds.
 */
void EddystoneTLM::setTime(uint32_t tmil) {
  m_eddystoneData.tmil = tmil;
}// setTime

}

#endif
