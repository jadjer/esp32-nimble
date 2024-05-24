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
#if defined(CONFIG_BT_ENABLED) && defined(CONFIG_BT_NIMBLE_ROLE_BROADCASTER)

#include "NimBLECharacteristic.h"
#include "NimBLEService.h"
#include "NimBLEDescriptor.h"
#include "HIDTypes.h"

#define GENERIC_HID		0x03C0
#define HID_KEYBOARD	   0x03C1
#define HID_MOUSE		  0x03C2
#define HID_JOYSTICK	   0x03C3
#define HID_GAMEPAD		0x03C4
#define HID_TABLET		 0x03C5
#define HID_CARD_READER	0x03C6
#define HID_DIGITAL_PEN	0x03C7
#define HID_BARCODE		0x03C8


/**
 * @brief A model of a %BLE Human Interface Device.
 */
class NimBLEHIDDevice {
public:
	NimBLEHIDDevice(NimBLEServer*);
	virtual ~NimBLEHIDDevice();

	void reportMap(uint8_t* map, uint16_t);
	void startServices();

	NimBLEService* deviceInfo();
	NimBLEService* hidService();
	NimBLEService* batteryService();

	NimBLECharacteristic* 	manufacturer();
	void 	manufacturer(std::string name);
	//NimBLECharacteristic* 	pnp();
	void	pnp(uint8_t sig, uint16_t vid, uint16_t pid, uint16_t version);
	//NimBLECharacteristic*	hidInfo();
	void	hidInfo(uint8_t country, uint8_t flags);
	NimBLECharacteristic* 	batteryLevel();
	void 	setBatteryLevel(uint8_t level);


	//NimBLECharacteristic* 	reportMap();
	NimBLECharacteristic* 	hidControl();
	NimBLECharacteristic* 	inputReport(uint8_t reportID);
	NimBLECharacteristic* 	outputReport(uint8_t reportID);
	NimBLECharacteristic* 	featureReport(uint8_t reportID);
	NimBLECharacteristic* 	protocolMode();
	NimBLECharacteristic* 	bootInput();
	NimBLECharacteristic* 	bootOutput();

private:
	NimBLEService*			m_deviceInfoService;			//0x180a
	NimBLEService*			m_hidService;					//0x1812
	NimBLEService*			m_batteryService = 0;			//0x180f

	NimBLECharacteristic* 	m_manufacturerCharacteristic;	//0x2a29
	NimBLECharacteristic* 	m_pnpCharacteristic;			//0x2a50
	NimBLECharacteristic* 	m_hidInfoCharacteristic;		//0x2a4a
	NimBLECharacteristic* 	m_reportMapCharacteristic;		//0x2a4b
	NimBLECharacteristic* 	m_hidControlCharacteristic;		//0x2a4c
	NimBLECharacteristic* 	m_protocolModeCharacteristic;	//0x2a4e
	NimBLECharacteristic*	m_batteryLevelCharacteristic;	//0x2a19
};

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_BROADCASTER */
