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

#include "nimble/Descriptor.hpp"
#include "nimble/Log.hpp"
#include "nimble/Service.hpp"

#include <string>

namespace nimble {

#define NULL_HANDLE (0xffff)

static const char *LOG_TAG = "NimBLEDescriptor";
static DescriptorCallbacks defaultCallbacks;

/**
 * @brief Construct a descriptor
 * @param [in] uuid - UUID (const char*) for the descriptor.
 * @param [in] properties - Properties for the descriptor.
 * @param [in] max_len - The maximum length in bytes that the descriptor value can hold. (Default: 512 bytes for esp32, 20 for all others).
 * @param [in] pCharacteristic - pointer to the characteristic instance this descriptor belongs to.
 */
Descriptor::Descriptor(const char *uuid, uint16_t properties, uint16_t max_len,
                       Characteristic *pCharacteristic)
    : Descriptor(UUID(uuid), properties, max_len, pCharacteristic) {
}

/**
 * @brief Construct a descriptor
 * @param [in] uuid - UUID (const char*) for the descriptor.
 * @param [in] properties - Properties for the descriptor.
 * @param [in] max_len - The maximum length in bytes that the descriptor value can hold. (Default: 512 bytes for esp32, 20 for all others).
 * @param [in] pCharacteristic - pointer to the characteristic instance this descriptor belongs to.
 */
Descriptor::Descriptor(UUID uuid, uint16_t properties, uint16_t max_len,
                       Characteristic *pCharacteristic)
    : m_value(std::min(CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH, (int) max_len), max_len) {
  m_uuid = uuid;
  m_handle = NULL_HANDLE;// Handle is initially unknown.
  m_pCharacteristic = pCharacteristic;
  m_pCallbacks = &defaultCallbacks;// No initial callback.
  m_properties = 0;
  m_removed = 0;

  if (properties & BLE_GATT_CHR_F_READ) {// convert uint16_t properties to uint8_t
    m_properties |= BLE_ATT_F_READ;
  }
  if (properties & (BLE_GATT_CHR_F_WRITE_NO_RSP | BLE_GATT_CHR_F_WRITE)) {
    m_properties |= BLE_ATT_F_WRITE;
  }
  if (properties & BLE_GATT_CHR_F_READ_ENC) {
    m_properties |= BLE_ATT_F_READ_ENC;
  }
  if (properties & BLE_GATT_CHR_F_READ_AUTHEN) {
    m_properties |= BLE_ATT_F_READ_AUTHEN;
  }
  if (properties & BLE_GATT_CHR_F_READ_AUTHOR) {
    m_properties |= BLE_ATT_F_READ_AUTHOR;
  }
  if (properties & BLE_GATT_CHR_F_WRITE_ENC) {
    m_properties |= BLE_ATT_F_WRITE_ENC;
  }
  if (properties & BLE_GATT_CHR_F_WRITE_AUTHEN) {
    m_properties |= BLE_ATT_F_WRITE_AUTHEN;
  }
  if (properties & BLE_GATT_CHR_F_WRITE_AUTHOR) {
    m_properties |= BLE_ATT_F_WRITE_AUTHOR;
  }

}// NimBLEDescriptor

/**
 * @brief NimBLEDescriptor destructor.
 */
Descriptor::~Descriptor() {
}// ~NimBLEDescriptor

/**
 * @brief Get the BLE handle for this descriptor.
 * @return The handle for this descriptor.
 */
uint16_t Descriptor::getHandle() {
  return m_handle;
}// getHandle

/**
 * @brief Get the length of the value of this descriptor.
 * @return The length (in bytes) of the value of this descriptor.
 */
size_t Descriptor::getLength() {
  return m_value.size();
}// getLength

/**
 * @brief Get the UUID of the descriptor.
 */
UUID Descriptor::getUUID() {
  return m_uuid;
}// getUUID

/**
 * @brief Get the value of this descriptor.
 * @return The NimBLEAttValue of this descriptor.
 */
AttributeValue Descriptor::getValue(time_t *timestamp) {
  if (timestamp != nullptr) {
    m_value.getValue(timestamp);
  }

  return m_value;
}// getValue

/**
 * @brief Get the value of this descriptor as a string.
 * @return A std::string instance containing a copy of the descriptor's value.
 */
std::string Descriptor::getStringValue() {
  return std::string(m_value);
}

/**
 * @brief Get the characteristic this descriptor belongs to.
 * @return A pointer to the characteristic this descriptor belongs to.
 */
Characteristic *Descriptor::getCharacteristic() {
  return m_pCharacteristic;
}// getCharacteristic

int Descriptor::handleGapEvent(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg) {
  (void) conn_handle;
  (void) attr_handle;

  const ble_uuid_t *uuid;
  int rc;
  ConnectionInfo peerInfo;
  Descriptor *pDescriptor = (Descriptor *) arg;

  NIMBLE_LOGD(LOG_TAG, "Descriptor %s %s event", pDescriptor->getUUID().toString().c_str(),
              ctxt->op == BLE_GATT_ACCESS_OP_READ_DSC ? "Read" : "Write");

  uuid = ctxt->chr->uuid;
  if (ble_uuid_cmp(uuid, &pDescriptor->getUUID().getNative()->u) == 0) {
    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_DSC: {
      rc = ble_gap_conn_find(conn_handle, &peerInfo.m_desc);
      assert(rc == 0);

      // If the packet header is only 8 bytes this is a follow up of a long read
      // so we don't want to call the onRead() callback again.
      if (ctxt->om->om_pkthdr_len > 8 || pDescriptor->m_value.size() <= (ble_att_mtu(peerInfo.getConnHandle()) - 3)) {
        pDescriptor->m_pCallbacks->onRead(pDescriptor, peerInfo);
      }

      ble_npl_hw_enter_critical();
      rc = os_mbuf_append(ctxt->om, pDescriptor->m_value.data(), pDescriptor->m_value.size());
      ble_npl_hw_exit_critical(0);
      return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    case BLE_GATT_ACCESS_OP_WRITE_DSC: {
      rc = ble_gap_conn_find(conn_handle, &peerInfo.m_desc);
      assert(rc == 0);

      uint16_t att_max_len = pDescriptor->m_value.max_size();

      if (ctxt->om->om_len > att_max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
      }

      uint8_t buf[att_max_len];
      size_t len = ctxt->om->om_len;
      memcpy(buf, ctxt->om->om_data, len);
      os_mbuf *next;
      next = SLIST_NEXT(ctxt->om, om_next);
      while (next != NULL) {
        if ((len + next->om_len) > att_max_len) {
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        memcpy(&buf[len], next->om_data, next->om_len);
        len += next->om_len;
        next = SLIST_NEXT(next, om_next);
      }

      pDescriptor->setValue(buf, len);
      pDescriptor->m_pCallbacks->onWrite(pDescriptor, peerInfo);
      return 0;
    }
    default:
      break;
    }
  }

  return BLE_ATT_ERR_UNLIKELY;
}

/**
 * @brief Set the callback handlers for this descriptor.
 * @param [in] pCallbacks An instance of a callback structure used to define any callbacks for the descriptor.
 */
void Descriptor::setCallbacks(DescriptorCallbacks *pCallbacks) {
  if (pCallbacks != nullptr) {
    m_pCallbacks = pCallbacks;
  } else {
    m_pCallbacks = &defaultCallbacks;
  }
}// setCallbacks

/**
 * @brief Set the handle of this descriptor.
 * Set the handle of this descriptor to be the supplied value.
 * @param [in] handle The handle to be associated with this descriptor.
 * @return N/A.
 */
void Descriptor::setHandle(uint16_t handle) {
  NIMBLE_LOGD(LOG_TAG, ">> setHandle(0x%.2x): Setting descriptor handle to be 0x%.2x", handle, handle);
  m_handle = handle;
  NIMBLE_LOGD(LOG_TAG, "<< setHandle()");
}// setHandle

/**
 * @brief Set the value of the descriptor.
 * @param [in] data The data to set for the descriptor.
 * @param [in] length The length of the data in bytes.
 */
void Descriptor::setValue(const uint8_t *data, size_t length) {
  m_value.setValue(data, length);
}// setValue

/**
 * @brief Set the value of the descriptor from a `std::vector<uint8_t>`.\n
 * @param [in] vec The std::vector<uint8_t> reference to set the descriptor value from.
 */
void Descriptor::setValue(const std::vector<uint8_t> &vec) {
  return setValue((uint8_t *) &vec[0], vec.size());
}// setValue

/**
 * @brief Set the characteristic this descriptor belongs to.
 * @param [in] pChar A pointer to the characteristic this descriptor belongs to.
 */
void Descriptor::setCharacteristic(Characteristic *pChar) {
  m_pCharacteristic = pChar;
}// setCharacteristic

/**
 * @brief Return a string representation of the descriptor.
 * @return A string representation of the descriptor.
 */
std::string Descriptor::toString() {
  char hex[5];
  snprintf(hex, sizeof(hex), "%04x", m_handle);
  std::string res = "UUID: " + m_uuid.toString() + ", handle: 0x" + hex;
  return res;
}// toString

/**
 * @brief Callback function to support a read request.
 * @param [in] pDescriptor The descriptor that is the source of the event.
 * @param [in] connInfo A reference to a NimBLEConnInfo instance containing the peer info.
 */
void DescriptorCallbacks::onRead(Descriptor *pDescriptor, ConnectionInfo &connInfo) {
  (void) pDescriptor;
  NIMBLE_LOGD("NimBLEDescriptorCallbacks", "onRead: default");
}// onRead

/**
 * @brief Callback function to support a write request.
 * @param [in] pDescriptor The descriptor that is the source of the event.
 * @param [in] connInfo A reference to a NimBLEConnInfo instance containing the peer info.
 */
void DescriptorCallbacks::onWrite(Descriptor *pDescriptor, ConnectionInfo &connInfo) {
  (void) pDescriptor;
  NIMBLE_LOGD("NimBLEDescriptorCallbacks", "onWrite: default");
}// onWrite

}

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_PERIPHERAL */
