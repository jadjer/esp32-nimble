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

/*
 * See also:
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.characteristic_presentation_format.xml
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_NIMBLE_ENABLED) && defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)

#include "nimble/Descriptor2904.hpp"

namespace nimble {

Descriptor2904::Descriptor2904(Characteristic *pCharacteristic) : Descriptor(UUID((uint16_t) 0x2904), BLE_GATT_CHR_F_READ, sizeof(BLE2904_Data), pCharacteristic) {
  m_data.m_format = 0;
  m_data.m_exponent = 0;
  m_data.m_namespace = 1;// 1 = Bluetooth SIG Assigned Numbers
  m_data.m_unit = 0;
  m_data.m_description = 0;
  setValue((uint8_t *) &m_data, sizeof(m_data));
}// BLE2904

/**
 * @brief Set the description.
 */
void Descriptor2904::setDescription(uint16_t description) {
  m_data.m_description = description;
  setValue((uint8_t *) &m_data, sizeof(m_data));
}

/**
 * @brief Set the exponent.
 */
void Descriptor2904::setExponent(int8_t exponent) {
  m_data.m_exponent = exponent;
  setValue((uint8_t *) &m_data, sizeof(m_data));
}// setExponent

/**
 * @brief Set the format.
 */
void Descriptor2904::setFormat(uint8_t format) {
  m_data.m_format = format;
  setValue((uint8_t *) &m_data, sizeof(m_data));
}// setFormat

/**
 * @brief Set the namespace.
 */
void Descriptor2904::setNamespace(uint8_t namespace_value) {
  m_data.m_namespace = namespace_value;
  setValue((uint8_t *) &m_data, sizeof(m_data));
}// setNamespace

/**
 * @brief Set the units for this value.  It should be one of the encoded values defined here:
 * https://www.bluetooth.com/specifications/assigned-numbers/units
 * @param [in] unit The type of units of this characteristic as defined by assigned numbers.
 */
void Descriptor2904::setUnit(uint16_t unit) {
  m_data.m_unit = unit;
  setValue((uint8_t *) &m_data, sizeof(m_data));
}// setUnit

}// namespace nimble

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_PERIPHERAL */
