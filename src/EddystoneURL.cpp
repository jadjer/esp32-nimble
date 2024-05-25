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

#include "nimble/EddystoneURL.hpp"
#include "nimble/Log.hpp"

#include <cstring>

static const char LOG_TAG[] = "NimBLEEddystoneURL";


namespace nimble {

/**
 * @brief Construct a default EddystoneURL beacon object.
 */
EddystoneURL::EddystoneURL() {
  beaconUUID = 0xFEAA;
  lengthURL = 0;
  m_eddystoneData.frameType = EDDYSTONE_URL_FRAME_TYPE;
  m_eddystoneData.advertisedTxPower = 0;
  memset(m_eddystoneData.url, 0, sizeof(m_eddystoneData.url));
}// BLEEddystoneURL

/**
 * @brief Retrieve the data that is being advertised.
 * @return The advertised data.
 */
std::string EddystoneURL::getData() {
  return std::string((char *) &m_eddystoneData, sizeof(m_eddystoneData));
}// getData

/**
 * @brief Get the UUID being advertised.
 * @return The UUID advertised.
 */
UUID EddystoneURL::getUUID() {
  return UUID(beaconUUID);
}// getUUID

/**
 * @brief Get the transmit power being advertised.
 * @return The transmit power.
 */
int8_t EddystoneURL::getPower() {
  return m_eddystoneData.advertisedTxPower;
}// getPower

/**
 * @brief Get the raw URL being advertised.
 * @return The raw URL.
 */
std::string EddystoneURL::getURL() {
  return std::string((char *) &m_eddystoneData.url, sizeof(m_eddystoneData.url));
}// getURL

/**
 * @brief Get the full URL being advertised.
 * @return The full URL.
 */
std::string EddystoneURL::getDecodedURL() {
  std::string decodedURL = "";

  switch (m_eddystoneData.url[0]) {
  case 0x00:
    decodedURL += "http://www.";
    break;
  case 0x01:
    decodedURL += "https://www.";
    break;
  case 0x02:
    decodedURL += "http://";
    break;
  case 0x03:
    decodedURL += "https://";
    break;
  default:
    decodedURL += m_eddystoneData.url[0];
  }

  for (int i = 1; i < lengthURL; i++) {
    if (m_eddystoneData.url[i] > 33 && m_eddystoneData.url[i] < 127) {
      decodedURL += m_eddystoneData.url[i];
    } else {
      switch (m_eddystoneData.url[i]) {
      case 0x00:
        decodedURL += ".com/";
        break;
      case 0x01:
        decodedURL += ".org/";
        break;
      case 0x02:
        decodedURL += ".edu/";
        break;
      case 0x03:
        decodedURL += ".net/";
        break;
      case 0x04:
        decodedURL += ".info/";
        break;
      case 0x05:
        decodedURL += ".biz/";
        break;
      case 0x06:
        decodedURL += ".gov/";
        break;
      case 0x07:
        decodedURL += ".com";
        break;
      case 0x08:
        decodedURL += ".org";
        break;
      case 0x09:
        decodedURL += ".edu";
        break;
      case 0x0A:
        decodedURL += ".net";
        break;
      case 0x0B:
        decodedURL += ".info";
        break;
      case 0x0C:
        decodedURL += ".biz";
        break;
      case 0x0D:
        decodedURL += ".gov";
        break;
      default:
        break;
      }
    }
  }
  return decodedURL;
}// getDecodedURL

/**
 * @brief Set the raw data for the beacon advertisement.
 * @param [in] data The raw data to advertise.
 */
void EddystoneURL::setData(const std::string &data) {
  if (data.length() > sizeof(m_eddystoneData)) {
    NIMBLE_LOGE(LOG_TAG, "Unable to set the data ... length passed in was %d and max expected %d",
                data.length(), sizeof(m_eddystoneData));
    return;
  }
  memset(&m_eddystoneData, 0, sizeof(m_eddystoneData));
  memcpy(&m_eddystoneData, data.data(), data.length());
  lengthURL = data.length() - (sizeof(m_eddystoneData) - sizeof(m_eddystoneData.url));
}// setData

/**
 * @brief Set the UUID to advertise.
 * @param [in] l_uuid The UUID.
 */
void EddystoneURL::setUUID(const UUID &l_uuid) {
  beaconUUID = l_uuid.getNative()->u16.value;
}// setUUID

/**
 * @brief Set the transmit power to advertise.
 * @param [in] advertisedTxPower The transmit power level.
 */
void EddystoneURL::setPower(int8_t advertisedTxPower) {
  m_eddystoneData.advertisedTxPower = advertisedTxPower;
}// setPower

/**
 * @brief Set the URL to advertise.
 * @param [in] url The URL.
 */
void EddystoneURL::setURL(const std::string &url) {
  if (url.length() > sizeof(m_eddystoneData.url)) {
    NIMBLE_LOGE(LOG_TAG, "Unable to set the url ... length passed in was %d and max expected %d",
                url.length(), sizeof(m_eddystoneData.url));
    return;
  }
  memset(m_eddystoneData.url, 0, sizeof(m_eddystoneData.url));
  memcpy(m_eddystoneData.url, url.data(), url.length());
  lengthURL = url.length();
}// setURL

}

#endif
