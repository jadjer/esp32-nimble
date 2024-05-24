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
#include "host/ble_uuid.h"
#else
#include "nimble/nimble/host/include/host/ble_uuid.h"
#endif

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

#include <string>

/**
 * @brief A model of a %BLE UUID.
 */
class NimBLEUUID {
public:
    NimBLEUUID(const std::string &uuid);
    NimBLEUUID(uint16_t uuid);
    NimBLEUUID(uint32_t uuid);
    NimBLEUUID(const ble_uuid128_t* uuid);
    NimBLEUUID(const uint8_t* pData, size_t size, bool msbFirst);
    NimBLEUUID(uint32_t first, uint16_t second, uint16_t third, uint64_t fourth);
    NimBLEUUID();

    uint8_t               bitSize() const;
    bool                  equals(const NimBLEUUID &uuid) const;
    const ble_uuid_any_t* getNative() const;
    const NimBLEUUID &    to128();
    const NimBLEUUID&     to16();
    std::string           toString() const;
    static NimBLEUUID     fromString(const std::string &uuid);

    bool operator ==(const NimBLEUUID & rhs) const;
    bool operator !=(const NimBLEUUID & rhs) const;
    operator std::string() const;

private:
    ble_uuid_any_t m_uuid;
    bool           m_valueSet = false;
}; // NimBLEUUID
#endif /* CONFIG_BT_ENABLED */
