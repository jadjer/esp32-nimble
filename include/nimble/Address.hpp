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

#include "nimble/ble.h"

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

#include <string>
#include <algorithm>

namespace nimble {

/**
 * @brief A %BLE device address.
 *
 * Every %BLE device has a unique address which can be used to identify it and form connections.
 */
class Address {
public:
  Address();
  explicit Address(ble_addr_t address);
  explicit Address(uint8_t address[6], uint8_t type = BLE_ADDR_PUBLIC);
  explicit Address(const std::string &stringAddress, uint8_t type = BLE_ADDR_PUBLIC);
  explicit Address(const uint64_t &address, uint8_t type = BLE_ADDR_PUBLIC);

public:
  [[nodiscard]] bool equals(const Address &otherAddress) const;
  [[nodiscard]] const uint8_t *getNative() const;
  [[nodiscard]] std::string toString() const;
  [[nodiscard]] uint8_t getType() const;

public:
  bool operator==(const Address &rhs) const;
  bool operator!=(const Address &rhs) const;
  explicit operator std::string() const;
  explicit operator uint64_t() const;

private:
  uint8_t m_address[6];
  uint8_t m_addrType;
};

}

#endif /* CONFIG_BT_ENABLED */
