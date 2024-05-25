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
#if defined(CONFIG_BT_NIMBLE_ENABLED) && defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)

#include "Server.hpp"
#include "nimble/Characteristic.hpp"
#include "nimble/Descriptor.hpp"
#include "nimble/HIDTypes.hpp"
#include "nimble/Server.hpp"
#include "nimble/Service.hpp"

namespace nimble {

#define GENERIC_HID 0x03C0
#define HID_KEYBOARD 0x03C1
#define HID_MOUSE 0x03C2
#define HID_JOYSTICK 0x03C3
#define HID_GAMEPAD 0x03C4
#define HID_TABLET 0x03C5
#define HID_CARD_READER 0x03C6
#define HID_DIGITAL_PEN 0x03C7
#define HID_BARCODE 0x03C8

/**
 * @brief A model of a %BLE Human Interface Device.
 */
class HIDDevice {
public:
  HIDDevice(Server *);
  virtual ~HIDDevice();

  void reportMap(uint8_t *map, uint16_t);
  void startServices();

  Service *deviceInfo();
  Service *hidService();
  Service *batteryService();

  Characteristic *manufacturer();
  void manufacturer(std::string name);
  //NimBLECharacteristic* 	pnp();
  void pnp(uint8_t sig, uint16_t vid, uint16_t pid, uint16_t version);
  //NimBLECharacteristic*	hidInfo();
  void hidInfo(uint8_t country, uint8_t flags);
  Characteristic *batteryLevel();
  void setBatteryLevel(uint8_t level);

  //NimBLECharacteristic* 	reportMap();
  Characteristic *hidControl();
  Characteristic *inputReport(uint8_t reportID);
  Characteristic *outputReport(uint8_t reportID);
  Characteristic *featureReport(uint8_t reportID);
  Characteristic *protocolMode();
  Characteristic *bootInput();
  Characteristic *bootOutput();

private:
  Service *m_deviceInfoService; //0x180a
  Service *m_hidService;        //0x1812
  Service *m_batteryService = 0;//0x180f

  Characteristic *m_manufacturerCharacteristic;//0x2a29
  Characteristic *m_pnpCharacteristic;         //0x2a50
  Characteristic *m_hidInfoCharacteristic;     //0x2a4a
  Characteristic *m_reportMapCharacteristic;   //0x2a4b
  Characteristic *m_hidControlCharacteristic;  //0x2a4c
  Characteristic *m_protocolModeCharacteristic;//0x2a4e
  Characteristic *m_batteryLevelCharacteristic;//0x2a19
};

}

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_BROADCASTER */
