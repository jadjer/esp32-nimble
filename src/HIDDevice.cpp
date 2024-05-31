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
#if defined(CONFIG_BT_NIMBLE_ENABLED) && defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)

#include "nimble/Descriptor2904.hpp"
#include "nimble/HIDDevice.hpp"

namespace nimble {

/**
 * @brief Construct a default NimBLEHIDDevice object.
 * @param [in] server A pointer to the server instance this HID Device will use.
 */
HIDDevice::HIDDevice(Server *server) {
  /*
	 * Here we create mandatory services described in bluetooth specification
	 */
  m_deviceInfoService = server->createService(UUID((uint16_t) 0x180a));
  m_hidService = server->createService(UUID((uint16_t) 0x1812));
  m_batteryService = server->createService(UUID((uint16_t) 0x180f));

  /*
	 * Mandatory characteristic for device info service
	 */
  m_pnpCharacteristic = m_deviceInfoService->createCharacteristic((uint16_t) 0x2a50, Property::READ);

  /*
	 * Mandatory characteristics for HID service
	 */
  m_hidInfoCharacteristic = m_hidService->createCharacteristic((uint16_t) 0x2a4a, Property::READ);
  m_reportMapCharacteristic = m_hidService->createCharacteristic((uint16_t) 0x2a4b, Property::READ);
  m_hidControlCharacteristic = m_hidService->createCharacteristic((uint16_t) 0x2a4c, Property::WRITE_NR);
  m_protocolModeCharacteristic = m_hidService->createCharacteristic((uint16_t) 0x2a4e, Property::WRITE_NR | Property::READ);

  /*
	 * Mandatory battery level characteristic with notification and presence descriptor
	 */
  m_batteryLevelCharacteristic = m_batteryService->createCharacteristic((uint16_t) 0x2a19, Property::READ | Property::NOTIFY);
  Descriptor2904 *batteryLevelDescriptor = (Descriptor2904 *) m_batteryLevelCharacteristic->createDescriptor((uint16_t) 0x2904);
  batteryLevelDescriptor->setFormat(Descriptor2904::FORMAT_UINT8);
  batteryLevelDescriptor->setNamespace(1);
  batteryLevelDescriptor->setUnit(0x27ad);

  /*
	 * This value is setup here because its default value in most usage cases, its very rare to use boot mode
	 * and we want to simplify library using as much as possible
	 */
  const uint8_t pMode[] = {0x01};
  protocolMode()->setValue((uint8_t *) pMode, 1);
}

/**
 * @brief Set the report map data formatting information.
 * @param [in] map A pointer to an array with the values to set.
 * @param [in] size The number of values in the array.
 */
void HIDDevice::reportMap(uint8_t *map, uint16_t size) {
  m_reportMapCharacteristic->setValue(map, size);
}

/**
 * @brief Start the HID device services.\n
 * This function called when all the services have been created.
 */
void HIDDevice::startServices() {
  m_deviceInfoService->start();
  m_hidService->start();
  m_batteryService->start();
}

/**
 * @brief Create a manufacturer characteristic (this characteristic is optional).
 */
Characteristic *HIDDevice::manufacturer() {
  m_manufacturerCharacteristic = m_deviceInfoService->createCharacteristic((uint16_t) 0x2a29, Property::READ);
  return m_manufacturerCharacteristic;
}

/**
 * @brief Set manufacturer name
 * @param [in] name The manufacturer name of this HID device.
 */
void HIDDevice::manufacturer(std::string name) {
  m_manufacturerCharacteristic->setValue(name);
}

/**
 * @brief Sets the Plug n Play characteristic value.
 * @param [in] sig The vendor ID source number.
 * @param [in] vid The vendor ID number.
 * @param [in] pid The product ID number.
 * @param [in] version The produce version number.
 */
void HIDDevice::pnp(uint8_t sig, uint16_t vid, uint16_t pid, uint16_t version) {
  uint8_t pnp[] = {sig, (uint8_t) (vid >> 8), (uint8_t) vid, (uint8_t) (pid >> 8), (uint8_t) pid, (uint8_t) (version >> 8), (uint8_t) version};
  m_pnpCharacteristic->setValue(pnp, sizeof(pnp));
}

/**
 * @brief Sets the HID Information characteristic value.
 * @param [in] country The country code for the device.
 * @param [in] flags The HID Class Specification release number to use.
 */
void HIDDevice::hidInfo(uint8_t country, uint8_t flags) {
  uint8_t info[] = {0x11, 0x1, country, flags};
  m_hidInfoCharacteristic->setValue(info, sizeof(info));
}

/**
 * @brief Create input report characteristic
 * @param [in] reportID input report ID, the same as in report map for input object related to the characteristic
 * @return pointer to new input report characteristic
 */
Characteristic *HIDDevice::inputReport(uint8_t reportID) {
  Characteristic *inputReportCharacteristic = m_hidService->createCharacteristic((uint16_t) 0x2a4d, Property::READ | Property::NOTIFY | Property::READ_ENC);
  Descriptor *inputReportDescriptor = inputReportCharacteristic->createDescriptor((uint16_t) 0x2908, Property::READ | Property::READ_ENC);

  uint8_t desc1_val[] = {reportID, 0x01};
  inputReportDescriptor->setValue((uint8_t *) desc1_val, 2);

  return inputReportCharacteristic;
}

/**
 * @brief Create output report characteristic
 * @param [in] reportID Output report ID, the same as in report map for output object related to the characteristic
 * @return Pointer to new output report characteristic
 */
Characteristic *HIDDevice::outputReport(uint8_t reportID) {
  Characteristic *outputReportCharacteristic = m_hidService->createCharacteristic((uint16_t) 0x2a4d, Property::READ | Property::WRITE | Property::WRITE_NR | Property::READ_ENC | Property::WRITE_ENC);
  Descriptor *outputReportDescriptor = outputReportCharacteristic->createDescriptor((uint16_t) 0x2908, Property::READ | Property::WRITE | Property::READ_ENC | Property::WRITE_ENC);

  uint8_t desc1_val[] = {reportID, 0x02};
  outputReportDescriptor->setValue((uint8_t *) desc1_val, 2);

  return outputReportCharacteristic;
}

/**
 * @brief Create feature report characteristic.
 * @param [in] reportID Feature report ID, the same as in report map for feature object related to the characteristic
 * @return Pointer to new feature report characteristic
 */
Characteristic *HIDDevice::featureReport(uint8_t reportID) {
  Characteristic *featureReportCharacteristic = m_hidService->createCharacteristic((uint16_t) 0x2a4d, Property::READ | Property::WRITE | Property::READ_ENC | Property::WRITE_ENC);
  Descriptor *featureReportDescriptor = featureReportCharacteristic->createDescriptor((uint16_t) 0x2908, Property::READ | Property::WRITE | Property::READ_ENC | Property::WRITE_ENC);

  uint8_t desc1_val[] = {reportID, 0x03};
  featureReportDescriptor->setValue((uint8_t *) desc1_val, 2);

  return featureReportCharacteristic;
}

/**
 * @brief Creates a keyboard boot input report characteristic
 */
Characteristic *HIDDevice::bootInput() {
  return m_hidService->createCharacteristic((uint16_t) 0x2a22, Property::NOTIFY);
}

/**
 * @brief Create a keyboard boot output report characteristic
 */
Characteristic *HIDDevice::bootOutput() {
  return m_hidService->createCharacteristic(static_cast<uint16_t>(0x2a32), Property::READ | Property::WRITE | Property::WRITE_NR);
}

/**
 * @brief Returns a pointer to the HID control point characteristic.
 */
Characteristic *HIDDevice::hidControl() {
  return m_hidControlCharacteristic;
}

/**
 * @brief Returns a pointer to the protocol mode characteristic.
 */
Characteristic *HIDDevice::protocolMode() {
  return m_protocolModeCharacteristic;
}

/**
 * @brief Set the battery level characteristic value.
 * @param [in] level The battery level value.
 */
void HIDDevice::setBatteryLevel(uint8_t level) {
  m_batteryLevelCharacteristic->setValue(&level, 1);
}
/*
 * @brief Returns battery level characteristic
 * @ return battery level characteristic
 */
Characteristic *HIDDevice::batteryLevel() {
  return m_batteryLevelCharacteristic;
}

/*

BLECharacteristic*	 BLEHIDDevice::reportMap() {
	return m_reportMapCharacteristic;
}

BLECharacteristic*	 BLEHIDDevice::pnp() {
	return m_pnpCharacteristic;
}


BLECharacteristic*	BLEHIDDevice::hidInfo() {
	return m_hidInfoCharacteristic;
}
*/

/**
 * @brief Returns a pointer to the device information service.
 */
Service *HIDDevice::deviceInfo() {
  return m_deviceInfoService;
}

/**
 * @brief Returns a pointer to the HID service.
 */
Service *HIDDevice::hidService() {
  return m_hidService;
}

/**
 * @brief @brief Returns a pointer to the battery service.
 */
[[maybe_unused]] Service *HIDDevice::batteryService() {
  return m_batteryService;
}

}// namespace nimble

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_PERIPHERAL */
