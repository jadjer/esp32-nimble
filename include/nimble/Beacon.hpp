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

#include "nimble/UUID.hpp"

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

namespace nimble {

/**
 * @brief Representation of a beacon.
 * See:
 * * https://en.wikipedia.org/wiki/IBeacon
 */
class Beacon {
private:
  struct {
    uint16_t manufacturerId;
    uint8_t subType;
    uint8_t subTypeLength;
    uint8_t proximityUUID[16];
    uint16_t major;
    uint16_t minor;
    int8_t signalPower;
  } __attribute__((packed)) m_beaconData;

public:
  Beacon();
  std::string getData();
  uint16_t getMajor() const;
  uint16_t getMinor() const;
  uint16_t getManufacturerId() const;
  UUID getProximityUUID();
  int8_t getSignalPower() const;
  void setData(const std::string &data);
  void setMajor(uint16_t major);
  void setMinor(uint16_t minor);
  void setManufacturerId(uint16_t manufacturerId);
  void setProximityUUID(const UUID &uuid);
  void setSignalPower(int8_t signalPower);
};// NimBLEBeacon

}
