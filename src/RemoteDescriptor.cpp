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
#if defined(CONFIG_BT_NIMBLE_ENABLED) && defined(CONFIG_BT_NIMBLE_ROLE_CENTRAL)

#include "nimble/Log.hpp"
#include "nimble/RemoteDescriptor.hpp"
#include "nimble/Utils.hpp"

#include <climits>

static const char* LOG_TAG = "NimBLERemoteDescriptor";

namespace nimble {

/**
 * @brief Remote descriptor constructor.
 * @param [in] pRemoteCharacteristic A pointer to the Characteristic that this belongs to.
 * @param [in] dsc A pointer to the struct that contains the descriptor information.
 */
RemoteDescriptor::RemoteDescriptor(RemoteCharacteristic *pRemoteCharacteristic,
                                   const struct ble_gatt_dsc *dsc) {
  NIMBLE_LOGD(LOG_TAG, ">> NimBLERemoteDescriptor()");
  switch (dsc->uuid.u.type) {
  case BLE_UUID_TYPE_16:
    m_uuid = UUID(dsc->uuid.u16.value);
    break;
  case BLE_UUID_TYPE_32:
    m_uuid = UUID(dsc->uuid.u32.value);
    break;
  case BLE_UUID_TYPE_128:
    m_uuid = UUID(const_cast<ble_uuid128_t *>(&dsc->uuid.u128));
    break;
  default:
    break;
  }

  m_handle = dsc->handle;
  m_pRemoteCharacteristic = pRemoteCharacteristic;

  NIMBLE_LOGD(LOG_TAG, "<< NimBLERemoteDescriptor(): %s", m_uuid.toString().c_str());
}

/**
 * @brief Retrieve the handle associated with this remote descriptor.
 * @return The handle associated with this remote descriptor.
 */
uint16_t RemoteDescriptor::getHandle() {
  return m_handle;
}// getHandle

/**
 * @brief Get the characteristic that owns this descriptor.
 * @return The characteristic that owns this descriptor.
 */
RemoteCharacteristic *RemoteDescriptor::getRemoteCharacteristic() {
  return m_pRemoteCharacteristic;
}// getRemoteCharacteristic

/**
 * @brief Retrieve the UUID associated this remote descriptor.
 * @return The UUID associated this remote descriptor.
 */
UUID RemoteDescriptor::getUUID() {
  return m_uuid;
}// getUUID

/**
 * @brief Read the value of the remote descriptor.
 * @return The value of the remote descriptor.
 */
AttributeValue RemoteDescriptor::readValue() {
  NIMBLE_LOGD(LOG_TAG, ">> Descriptor readValue: %s", toString().c_str());

  Client *pClient = getRemoteCharacteristic()->getRemoteService()->getClient();
  AttributeValue value;

  if (!pClient->isConnected()) {
    NIMBLE_LOGE(LOG_TAG, "Disconnected");
    return value;
  }

  int rc = 0;
  int retryCount = 1;
  TaskHandle_t cur_task = xTaskGetCurrentTaskHandle();
  ble_task_data_t taskData = {this, cur_task, 0, &value};

  do {
    rc = ble_gattc_read_long(pClient->getConnId(), m_handle, 0,
                             RemoteDescriptor::onReadCB,
                             &taskData);
    if (rc != 0) {
      NIMBLE_LOGE(LOG_TAG, "Error: Failed to read descriptor; rc=%d, %s",
                  rc, Utils::returnCodeToString(rc));
      return value;
    }

#ifdef ulTaskNotifyValueClear
    // Clear the task notification value to ensure we block
    ulTaskNotifyValueClear(cur_task, ULONG_MAX);
#endif
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    rc = taskData.rc;

    switch (rc) {
    case 0:
    case BLE_HS_EDONE:
      rc = 0;
      break;
    // Descriptor is not long-readable, return with what we have.
    case BLE_HS_ATT_ERR(BLE_ATT_ERR_ATTR_NOT_LONG):
      NIMBLE_LOGI(LOG_TAG, "Attribute not long");
      rc = 0;
      break;
    case BLE_HS_ATT_ERR(BLE_ATT_ERR_INSUFFICIENT_AUTHEN):
    case BLE_HS_ATT_ERR(BLE_ATT_ERR_INSUFFICIENT_AUTHOR):
    case BLE_HS_ATT_ERR(BLE_ATT_ERR_INSUFFICIENT_ENC):
      if (retryCount && pClient->secureConnection())
        break;
    /* Else falls through. */
    default:
      return value;
    }
  } while (rc != 0 && retryCount--);

  NIMBLE_LOGD(LOG_TAG, "<< Descriptor readValue(): length: %u rc=%d", value.length(), rc);
  return value;
}// readValue

/**
 * @brief Callback for Descriptor read operation.
 * @return success == 0 or error code.
 */
int RemoteDescriptor::onReadCB(uint16_t conn_handle,
                               const struct ble_gatt_error *error,
                               struct ble_gatt_attr *attr, void *arg) {
  (void) attr;
  ble_task_data_t *pTaskData = (ble_task_data_t *) arg;
  RemoteDescriptor *desc = (RemoteDescriptor *) pTaskData->pATT;
  uint16_t conn_id = desc->getRemoteCharacteristic()->getRemoteService()->getClient()->getConnId();

  if (conn_id != conn_handle) {
    return 0;
  }

  NIMBLE_LOGD(LOG_TAG, "Read complete; status=%d conn_handle=%d", error->status, conn_handle);

  AttributeValue *valBuf = (AttributeValue *) pTaskData->buf;
  int rc = error->status;

  if (rc == 0) {
    if (attr) {
      uint16_t data_len = OS_MBUF_PKTLEN(attr->om);
      if ((valBuf->size() + data_len) > BLE_ATT_ATTR_MAX_LEN) {
        rc = BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
      } else {
        NIMBLE_LOGD(LOG_TAG, "Got %u bytes", data_len);
        valBuf->append(attr->om->om_data, data_len);
        return 0;
      }
    }
  }

  pTaskData->rc = rc;
  xTaskNotifyGive(pTaskData->task);

  return rc;
}

/**
 * @brief Return a string representation of this Remote Descriptor.
 * @return A string representation of this Remote Descriptor.
 */
std::string RemoteDescriptor::toString() {
  std::string res = "Descriptor: uuid: " + getUUID().toString();
  char val[6];
  res += ", handle: ";
  snprintf(val, sizeof(val), "%d", getHandle());
  res += val;

  return res;
}// toString

/**
 * @brief Callback for descriptor write operation.
 * @return success == 0 or error code.
 */
int RemoteDescriptor::onWriteCB(uint16_t conn_handle,
                                const struct ble_gatt_error *error,
                                struct ble_gatt_attr *attr, void *arg) {
  ble_task_data_t *pTaskData = (ble_task_data_t *) arg;
  RemoteDescriptor *descriptor = (RemoteDescriptor *) pTaskData->pATT;

  if (descriptor->getRemoteCharacteristic()->getRemoteService()->getClient()->getConnId() != conn_handle) {
    return 0;
  }

  NIMBLE_LOGI(LOG_TAG, "Write complete; status=%d conn_handle=%d", error->status, conn_handle);

  pTaskData->rc = error->status;
  xTaskNotifyGive(pTaskData->task);

  return 0;
}

/**
 * @brief Write a new value to a remote descriptor from a std::vector<uint8_t>.
 * @param [in] vec A std::vector<uint8_t> value to write to the remote descriptor.
 * @param [in] response Whether we require a response from the write.
 * @return false if not connected or otherwise cannot perform write.
 */
bool RemoteDescriptor::writeValue(const std::vector<uint8_t> &vec, bool response) {
  return writeValue((uint8_t *) &vec[0], vec.size(), response);
}// writeValue

/**
 * @brief Write a new value to the remote descriptor from a const char*.
 * @param [in] char_s A character string to write to the remote descriptor.
 * @param [in] response Whether we require a response from the write.
 * @return false if not connected or otherwise cannot perform write.
 */
bool RemoteDescriptor::writeValue(const char *char_s, bool response) {
  return writeValue((uint8_t *) char_s, strlen(char_s), response);
}// writeValue

/**
 * @brief Write a new value to a remote descriptor.
 * @param [in] data The data to send to the remote descriptor.
 * @param [in] length The length of the data to send.
 * @param [in] response True if we expect a write response.
 * @return false if not connected or otherwise cannot perform write.
 */
bool RemoteDescriptor::writeValue(const uint8_t *data, size_t length, bool response) {

  NIMBLE_LOGD(LOG_TAG, ">> Descriptor writeValue: %s", toString().c_str());

  Client *pClient = getRemoteCharacteristic()->getRemoteService()->getClient();

  // Check to see that we are connected.
  if (!pClient->isConnected()) {
    NIMBLE_LOGE(LOG_TAG, "Disconnected");
    return false;
  }

  int rc = 0;
  int retryCount = 1;
  uint16_t mtu = ble_att_mtu(pClient->getConnId()) - 3;

  // Check if the data length is longer than we can write in 1 connection event.
  // If so we must do a long write which requires a response.
  if (length <= mtu && !response) {
    rc = ble_gattc_write_no_rsp_flat(pClient->getConnId(), m_handle, data, length);
    return (rc == 0);
  }

  TaskHandle_t cur_task = xTaskGetCurrentTaskHandle();
  ble_task_data_t taskData = {this, cur_task, 0, nullptr};

  do {
    if (length > mtu) {
      NIMBLE_LOGI(LOG_TAG, "long write %d bytes", length);
      os_mbuf *om = ble_hs_mbuf_from_flat(data, length);
      rc = ble_gattc_write_long(pClient->getConnId(), m_handle, 0, om,
                                RemoteDescriptor::onWriteCB,
                                &taskData);
    } else {
      rc = ble_gattc_write_flat(pClient->getConnId(), m_handle,
                                data, length,
                                RemoteDescriptor::onWriteCB,
                                &taskData);
    }

    if (rc != 0) {
      NIMBLE_LOGE(LOG_TAG, "Error: Failed to write descriptor; rc=%d", rc);
      return false;
    }

#ifdef ulTaskNotifyValueClear
    // Clear the task notification value to ensure we block
    ulTaskNotifyValueClear(cur_task, ULONG_MAX);
#endif
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    rc = taskData.rc;

    switch (rc) {
    case 0:
    case BLE_HS_EDONE:
      rc = 0;
      break;
    case BLE_HS_ATT_ERR(BLE_ATT_ERR_ATTR_NOT_LONG):
      NIMBLE_LOGE(LOG_TAG, "Long write not supported by peer; Truncating length to %d", mtu);
      retryCount++;
      length = mtu;
      break;

    case BLE_HS_ATT_ERR(BLE_ATT_ERR_INSUFFICIENT_AUTHEN):
    case BLE_HS_ATT_ERR(BLE_ATT_ERR_INSUFFICIENT_AUTHOR):
    case BLE_HS_ATT_ERR(BLE_ATT_ERR_INSUFFICIENT_ENC):
      if (retryCount && pClient->secureConnection())
        break;
    /* Else falls through. */
    default:
      return false;
    }
  } while (rc != 0 && retryCount--);

  NIMBLE_LOGD(LOG_TAG, "<< Descriptor writeValue, rc: %d", rc);
  return (rc == 0);
}// writeValue

}

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_CENTRAL */
