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

#define EDDYSTONE_URL_FRAME_TYPE 0x10

/**
 * @brief Representation of a beacon.
 * See:
 * * https://github.com/google/eddystone
 */
class NimBLEEddystoneURL {
public:
    NimBLEEddystoneURL();
    std::string getData();
    NimBLEUUID   getUUID();
    int8_t    getPower();
    std::string getURL();
    std::string getDecodedURL();
    void        setData(const std::string &data);
    void        setUUID(const NimBLEUUID &l_uuid);
    void        setPower(int8_t advertisedTxPower);
    void        setURL(const std::string &url);

private:
    uint16_t beaconUUID;
    uint8_t  lengthURL;
    struct {
        uint8_t frameType;
        int8_t  advertisedTxPower;
        uint8_t url[16];
    } __attribute__((packed)) m_eddystoneData;

}; // NIMBLEEddystoneURL
