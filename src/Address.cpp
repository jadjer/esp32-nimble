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

#include <algorithm>

#include "nimble/Address.hpp"
#include "nimble/Log.hpp"
#include "nimble/Utils.hpp"

namespace nimble {

static const char *LOG_TAG = "NimBLEAddress";

/*************************************************
 * NOTE: NimBLE address bytes are in INVERSE ORDER!
 * We will accomodate that fact in these methods.
*************************************************/

/**
 * @brief Create an address from the native NimBLE representation.
 * @param [in] address The native NimBLE address.
 */
Address::Address(ble_addr_t address) {
  memcpy(m_address, address.val, 6);
  m_addrType = address.type;
}// NimBLEAddress

/**
 * @brief Create a blank address, i.e. 00:00:00:00:00:00, type 0.
 */
Address::Address() {
  Address("");
}// NimBLEAddress

/**
 * @brief Create an address from a hex string
 *
 * A hex string is of the format:
 * ```
 * 00:00:00:00:00:00
 * ```
 * which is 17 characters in length.
 *
 * @param [in] stringAddress The hex string representation of the address.
 * @param [in] type The type of the address.
 */
Address::Address(const std::string &stringAddress, uint8_t type) {
  m_addrType = type;

  if (stringAddress.length() == 0) {
    memset(m_address, 0, 6);
    return;
  }

  if (stringAddress.length() == 6) {
    std::reverse_copy(stringAddress.data(), stringAddress.data() + 6, m_address);
    return;
  }

  if (stringAddress.length() != 17) {
    memset(m_address, 0, sizeof m_address);// "00:00:00:00:00:00" represents an invalid address
    NIMBLE_LOGD(LOG_TAG, "Invalid address '%s'", stringAddress.c_str());
    return;
  }

  int data[6];
  if (sscanf(stringAddress.c_str(), "%x:%x:%x:%x:%x:%x", &data[5], &data[4], &data[3], &data[2], &data[1], &data[0]) != 6) {
    memset(m_address, 0, sizeof m_address);// "00:00:00:00:00:00" represents an invalid address
    NIMBLE_LOGD(LOG_TAG, "Invalid address '%s'", stringAddress.c_str());
  }
  for (size_t index = 0; index < sizeof m_address; index++) {
    m_address[index] = data[index];
  }
}// NimBLEAddress

/**
 * @brief Constructor for compatibility with bluedroid esp library using native ESP representation.
 * @param [in] address A uint8_t[6] or esp_bd_addr_t containing the address.
 * @param [in] type The type of the address.
 */
Address::Address(uint8_t address[6], uint8_t type) {
  std::reverse_copy(address, address + sizeof m_address, m_address);
  m_addrType = type;
}// NimBLEAddress

/**
 * @brief Constructor for address using a hex value.\n
 * Use the same byte order, so use 0xa4c1385def16 for "a4:c1:38:5d:ef:16"
 * @param [in] address uint64_t containing the address.
 * @param [in] type The type of the address.
 */
Address::Address(const uint64_t &address, uint8_t type) {
  memcpy(m_address, &address, sizeof m_address);
  m_addrType = type;
}// NimBLEAddress

/**
 * @brief Determine if this address equals another.
 * @param [in] otherAddress The other address to compare against.
 * @return True if the addresses are equal.
 */
bool Address::equals(const Address &otherAddress) const {
  return *this == otherAddress;
}// equals

/**
 * @brief Get the native representation of the address.
 * @return a pointer to the uint8_t[6] array of the address.
 */
const uint8_t *Address::getNative() const {
  return m_address;
}// getNative

/**
 * @brief Get the address type.
 * @return The address type.
 */
uint8_t Address::getType() const {
  return m_addrType;
}// getType

/**
 * @brief Convert a BLE address to a string.
 *
 * A string representation of an address is in the format:
 *
 * ```
 * xx:xx:xx:xx:xx:xx
 * ```
 *
 * @return The string representation of the address.
 * @deprecated Use std::string() operator instead.
 */
std::string Address::toString() const {
  return std::string(*this);
}// toString

/**
 * @brief Convenience operator to check if this address is equal to another.
 */
bool Address::operator==(const Address &rhs) const {
  return memcmp(rhs.m_address, m_address, sizeof m_address) == 0;
}// operator ==

/**
 * @brief Convenience operator to check if this address is not equal to another.
 */
bool Address::operator!=(const Address &rhs) const {
  return !this->operator==(rhs);
}// operator !=

/**
 * @brief Convienience operator to convert this address to string representation.
 * @details This allows passing NimBLEAddress to functions
 * that accept std::string and/or or it's methods as a parameter.
 */
Address::operator std::string() const {
  char buffer[18];
  snprintf(buffer, sizeof(buffer), "%02x:%02x:%02x:%02x:%02x:%02x",
           m_address[5], m_address[4], m_address[3],
           m_address[2], m_address[1], m_address[0]);
  return std::string(buffer);
}// operator std::string

/**
 * @brief Convenience operator to convert the native address representation to uint_64.
 */
Address::operator uint64_t() const {
  uint64_t address = 0;
  memcpy(&address, m_address, sizeof m_address);
  return address;
}// operator uint64_t

}

#endif