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

#include "NimBLEUUID.h"

#include <string>

#define EDDYSTONE_TLM_FRAME_TYPE 0x20

/**
 * @brief Representation of a beacon.
 * See:
 * * https://github.com/google/eddystone
 */
class NimBLEEddystoneTLM {
public:
    NimBLEEddystoneTLM();
    std::string getData();
    NimBLEUUID   getUUID();
    uint8_t  getVersion();
    uint16_t    getVolt();
    float      getTemp();
    uint32_t    getCount();
    uint32_t    getTime();
    std::string toString();
    void        setData(const std::string &data);
    void        setUUID(const NimBLEUUID &l_uuid);
    void        setVersion(uint8_t version);
    void        setVolt(uint16_t volt);
    void        setTemp(float temp);
    void        setCount(uint32_t advCount);
    void        setTime(uint32_t tmil);

private:
    uint16_t beaconUUID;
    struct {
        uint8_t frameType;
        uint8_t version;
        uint16_t volt;
        uint16_t temp;
        uint32_t advCount;
        uint32_t tmil;
    } __attribute__((packed)) m_eddystoneData;

}; // NimBLEEddystoneTLM
