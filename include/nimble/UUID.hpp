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

#include <string>
#include <host/ble_uuid.h>

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

namespace nimble {

/**
 * @brief A model of a %BLE UUID.
 */
class UUID {
public:
  explicit UUID(const std::string &uuid);
  explicit UUID(uint16_t uuid);
  explicit UUID(uint32_t uuid);
  explicit UUID(const ble_uuid128_t *uuid);
  UUID(const uint8_t *pData, size_t size, bool msbFirst);
  UUID(uint32_t first, uint16_t second, uint16_t third, uint64_t fourth);
  UUID();

public:
  [[nodiscard]] uint8_t bitSize() const;
  [[nodiscard]] bool equals(const UUID &uuid) const;
  [[nodiscard]] const ble_uuid_any_t *getNative() const;
  const UUID &to128();
  const UUID &to16();
  [[nodiscard]] std::string toString() const;
  static UUID fromString(const std::string &uuid);

public:
  bool operator==(const UUID &rhs) const;
  bool operator!=(const UUID &rhs) const;
  explicit operator std::string() const;

private:
  ble_uuid_any_t m_uuid {};
  bool m_valueSet = false;
};

}

#endif
