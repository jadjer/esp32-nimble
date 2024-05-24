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

#include "nimconfig.h"
#if defined(CONFIG_BT_ENABLED)

#if defined(CONFIG_NIMBLE_CPP_IDF)
#include "nimble/ble.h"
#else
#include "nimble/nimble/include/nimble/ble.h"
#endif

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

#include <string>
#include <algorithm>

/**
 * @brief A %BLE device address.
 *
 * Every %BLE device has a unique address which can be used to identify it and form connections.
 */
class NimBLEAddress {
public:
    NimBLEAddress();
    NimBLEAddress(ble_addr_t address);
    NimBLEAddress(uint8_t address[6], uint8_t type = BLE_ADDR_PUBLIC);
    NimBLEAddress(const std::string &stringAddress, uint8_t type = BLE_ADDR_PUBLIC);
    NimBLEAddress(const uint64_t &address, uint8_t type = BLE_ADDR_PUBLIC);
    bool            equals(const NimBLEAddress &otherAddress) const;
    const uint8_t*  getNative() const;
    std::string     toString() const;
    uint8_t         getType() const;

    bool operator   ==(const NimBLEAddress & rhs) const;
    bool operator   !=(const NimBLEAddress & rhs) const;
    operator        std::string() const;
    operator        uint64_t() const;

private:
    uint8_t        m_address[6];
    uint8_t        m_addrType;
};

#endif /* CONFIG_BT_ENABLED */
