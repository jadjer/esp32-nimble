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

#include "nimble/Device.hpp"
#include "nimble/Log.hpp"
#include "nimble/RemoteService.hpp"
#include "nimble/Utils.hpp"

#include <climits>

static const char* LOG_TAG = "NimBLERemoteService";

namespace nimble {

/**
 * @brief Remote Service constructor.
 * @param [in] pClient A pointer to the client this belongs to.
 * @param [in] service A pointer to the structure with the service information.
 */
RemoteService::RemoteService(Client *pClient, const struct ble_gatt_svc *service) {

  NIMBLE_LOGD(LOG_TAG, ">> NimBLERemoteService()");
  m_pClient = pClient;
  switch (service->uuid.u.type) {
  case BLE_UUID_TYPE_16:
    m_uuid = UUID(service->uuid.u16.value);
    break;
  case BLE_UUID_TYPE_32:
    m_uuid = UUID(service->uuid.u32.value);
    break;
  case BLE_UUID_TYPE_128:
    m_uuid = UUID(const_cast<ble_uuid128_t *>(&service->uuid.u128));
    break;
  default:
    break;
  }
  m_startHandle = service->start_handle;
  m_endHandle = service->end_handle;
  NIMBLE_LOGD(LOG_TAG, "<< NimBLERemoteService(): %s", m_uuid.toString().c_str());
}

/**
 * @brief When deleting the service make sure we delete all characteristics and descriptors.
 */
RemoteService::~RemoteService() {
  deleteCharacteristics();
}

/**
 * @brief Get iterator to the beginning of the vector of remote characteristic pointers.
 * @return An iterator to the beginning of the vector of remote characteristic pointers.
 */
std::vector<RemoteCharacteristic *>::iterator RemoteService::begin() {
  return m_characteristicVector.begin();
}

/**
 * @brief Get iterator to the end of the vector of remote characteristic pointers.
 * @return An iterator to the end of the vector of remote characteristic pointers.
 */
std::vector<RemoteCharacteristic *>::iterator RemoteService::end() {
  return m_characteristicVector.end();
}

/**
 * @brief Get the remote characteristic object for the characteristic UUID.
 * @param [in] uuid Remote characteristic uuid.
 * @return A pointer to the remote characteristic object.
 */
RemoteCharacteristic *RemoteService::getCharacteristic(const char *uuid) {
  return getCharacteristic(UUID(uuid));
}// getCharacteristic

/**
 * @brief Get the characteristic object for the UUID.
 * @param [in] uuid Characteristic uuid.
 * @return A pointer to the characteristic object, or nullptr if not found.
 */
RemoteCharacteristic *RemoteService::getCharacteristic(const UUID &uuid) {
  NIMBLE_LOGD(LOG_TAG, ">> getCharacteristic: uuid: %s", uuid.toString().c_str());

  for (auto &it : m_characteristicVector) {
    if (it->getUUID() == uuid) {
      NIMBLE_LOGD(LOG_TAG, "<< getCharacteristic: found the characteristic with uuid: %s", uuid.toString().c_str());
      return it;
    }
  }

  size_t prev_size = m_characteristicVector.size();
  if (retrieveCharacteristics(&uuid)) {
    if (m_characteristicVector.size() > prev_size) {
      return m_characteristicVector.back();
    }

    // If the request was successful but 16/32 bit uuid not found
    // try again with the 128 bit uuid.
    if (uuid.bitSize() == BLE_UUID_TYPE_16 || uuid.bitSize() == BLE_UUID_TYPE_32) {
      UUID uuid128(uuid);
      uuid128.to128();
      if (retrieveCharacteristics(&uuid128)) {
        if (m_characteristicVector.size() > prev_size) {
          return m_characteristicVector.back();
        }
      }
    } else {
      // If the request was successful but the 128 bit uuid not found
      // try again with the 16 bit uuid.
      UUID uuid16(uuid);
      uuid16.to16();
      // if the uuid was 128 bit but not of the BLE base type this check will fail
      if (uuid16.bitSize() == BLE_UUID_TYPE_16) {
        if (retrieveCharacteristics(&uuid16)) {
          if (m_characteristicVector.size() > prev_size) {
            return m_characteristicVector.back();
          }
        }
      }
    }
  }

  NIMBLE_LOGD(LOG_TAG, "<< getCharacteristic: not found");
  return nullptr;
}// getCharacteristic

/**
 * @brief Get a pointer to the vector of found characteristics.
 * @param [in] refresh If true the current characteristics vector will cleared and
 * all characteristics for this service retrieved from the peripheral.
 * If false the vector will be returned with the currently stored characteristics of this service.
 * @return A pointer to the vector of descriptors for this characteristic.
 */
std::vector<RemoteCharacteristic *> *RemoteService::getCharacteristics(bool refresh) {
  if (refresh) {
    deleteCharacteristics();

    if (!retrieveCharacteristics()) {
      NIMBLE_LOGE(LOG_TAG, "Error: Failed to get characteristics");
    } else {
      NIMBLE_LOGI(LOG_TAG, "Found %d characteristics", m_characteristicVector.size());
    }
  }
  return &m_characteristicVector;
}// getCharacteristics

/**
 * @brief Callback for Characteristic discovery.
 * @return success == 0 or error code.
 */
int RemoteService::characteristicDiscCB(uint16_t conn_handle,
                                        const struct ble_gatt_error *error,
                                        const struct ble_gatt_chr *chr, void *arg) {
  NIMBLE_LOGD(LOG_TAG, "Characteristic Discovered >> status: %d handle: %d",
              error->status, (error->status == 0) ? chr->val_handle : -1);

  ble_task_data_t *pTaskData = (ble_task_data_t *) arg;
  RemoteService *service = (RemoteService *) pTaskData->pATT;

  // Make sure the discovery is for this device
  if (service->getClient()->getConnId() != conn_handle) {
    return 0;
  }

  if (error->status == 0) {
    // Found a service - add it to the vector
    RemoteCharacteristic *pRemoteCharacteristic = new RemoteCharacteristic(service, chr);
    service->m_characteristicVector.push_back(pRemoteCharacteristic);
    return 0;
  }

  if (error->status == BLE_HS_EDONE) {
    pTaskData->rc = 0;
  } else {
    NIMBLE_LOGE(LOG_TAG, "characteristicDiscCB() rc=%d %s",
                error->status,
                Utils::returnCodeToString(error->status));
    pTaskData->rc = error->status;
  }

  xTaskNotifyGive(pTaskData->task);

  NIMBLE_LOGD(LOG_TAG, "<< Characteristic Discovered");
  return error->status;
}

/**
 * @brief Retrieve all the characteristics for this service.
 * This function will not return until we have all the characteristics.
 * @return True if successful.
 */
bool RemoteService::retrieveCharacteristics(const UUID *uuid_filter) {
  NIMBLE_LOGD(LOG_TAG, ">> retrieveCharacteristics() for service: %s", getUUID().toString().c_str());

  int rc = 0;
  TaskHandle_t cur_task = xTaskGetCurrentTaskHandle();
  ble_task_data_t taskData = {this, cur_task, 0, nullptr};

  if (uuid_filter == nullptr) {
    rc = ble_gattc_disc_all_chrs(m_pClient->getConnId(),
                                 m_startHandle,
                                 m_endHandle,
                                 RemoteService::characteristicDiscCB,
                                 &taskData);
  } else {
    rc = ble_gattc_disc_chrs_by_uuid(m_pClient->getConnId(),
                                     m_startHandle,
                                     m_endHandle,
                                     &uuid_filter->getNative()->u,
                                     RemoteService::characteristicDiscCB,
                                     &taskData);
  }

  if (rc != 0) {
    NIMBLE_LOGE(LOG_TAG, "ble_gattc_disc_all_chrs: rc=%d %s", rc, Utils::returnCodeToString(rc));
    return false;
  }

#ifdef ulTaskNotifyValueClear
  // Clear the task notification value to ensure we block
  ulTaskNotifyValueClear(cur_task, ULONG_MAX);
#endif
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

  if (taskData.rc == 0) {
    if (uuid_filter == nullptr) {
      if (m_characteristicVector.size() > 1) {
        for (auto it = m_characteristicVector.begin(); it != m_characteristicVector.end(); ++it) {
          auto nx = std::next(it, 1);
          if (nx == m_characteristicVector.end()) {
            break;
          }
          (*it)->m_endHandle = (*nx)->m_defHandle - 1;
        }
      }

      if (m_characteristicVector.size() > 0) {
        m_characteristicVector.back()->m_endHandle = getEndHandle();
      }
    }

    NIMBLE_LOGD(LOG_TAG, "<< retrieveCharacteristics()");
    return true;
  }

  NIMBLE_LOGE(LOG_TAG, "Could not retrieve characteristics");
  return false;

}// retrieveCharacteristics

/**
 * @brief Get the client associated with this service.
 * @return A reference to the client associated with this service.
 */
Client *RemoteService::getClient() {
  return m_pClient;
}// getClient

/**
 * @brief Get the service end handle.
 */
uint16_t RemoteService::getEndHandle() {
  return m_endHandle;
}// getEndHandle

/**
 * @brief Get the service start handle.
 */
uint16_t RemoteService::getStartHandle() {
  return m_startHandle;
}// getStartHandle

/**
 * @brief Get the service UUID.
 */
UUID RemoteService::getUUID() {
  return m_uuid;
}

/**
 * @brief Read the value of a characteristic associated with this service.
 * @param [in] characteristicUuid The characteristic to read.
 * @returns a string containing the value or an empty string if not found or error.
 */
std::string RemoteService::getValue(const UUID &characteristicUuid) {
  NIMBLE_LOGD(LOG_TAG, ">> readValue: uuid: %s", characteristicUuid.toString().c_str());

  std::string ret = "";
  RemoteCharacteristic *pChar = getCharacteristic(characteristicUuid);

  if (pChar != nullptr) {
    ret = pChar->readValue();
  }

  NIMBLE_LOGD(LOG_TAG, "<< readValue");
  return ret;
}// readValue

/**
 * @brief Set the value of a characteristic.
 * @param [in] characteristicUuid The characteristic to set.
 * @param [in] value The value to set.
 * @returns true on success, false if not found or error
 */
bool RemoteService::setValue(const UUID &characteristicUuid, const std::string &value) {
  NIMBLE_LOGD(LOG_TAG, ">> setValue: uuid: %s", characteristicUuid.toString().c_str());

  bool ret = false;
  RemoteCharacteristic *pChar = getCharacteristic(characteristicUuid);

  if (pChar != nullptr) {
    ret = pChar->writeValue(value);
  }

  NIMBLE_LOGD(LOG_TAG, "<< setValue");
  return ret;
}// setValue

/**
 * @brief Delete the characteristics in the characteristics vector.
 * @details We maintain a vector called m_characteristicsVector that contains pointers to BLERemoteCharacteristic
 * object references. Since we allocated these in this class, we are also responsible for deleting
 * them. This method does just that.
 */
void RemoteService::deleteCharacteristics() {
  NIMBLE_LOGD(LOG_TAG, ">> deleteCharacteristics");
  for (auto &it : m_characteristicVector) {
    delete it;
  }
  m_characteristicVector.clear();
  NIMBLE_LOGD(LOG_TAG, "<< deleteCharacteristics");
}// deleteCharacteristics

/**
 * @brief Delete characteristic by UUID
 * @param [in] uuid The UUID of the characteristic to be removed from the local database.
 * @return Number of characteristics left.
 */
size_t RemoteService::deleteCharacteristic(const UUID &uuid) {
  NIMBLE_LOGD(LOG_TAG, ">> deleteCharacteristic");

  for (auto it = m_characteristicVector.begin(); it != m_characteristicVector.end(); ++it) {
    if ((*it)->getUUID() == uuid) {
      delete *it;
      m_characteristicVector.erase(it);
      break;
    }
  }

  NIMBLE_LOGD(LOG_TAG, "<< deleteCharacteristic");

  return m_characteristicVector.size();
}// deleteCharacteristic

/**
 * @brief Create a string representation of this remote service.
 * @return A string representation of this remote service.
 */
std::string RemoteService::toString() {
  std::string res = "Service: uuid: " + m_uuid.toString();
  char val[6];
  res += ", start_handle: ";
  snprintf(val, sizeof(val), "%d", m_startHandle);
  res += val;
  snprintf(val, sizeof(val), "%04x", m_startHandle);
  res += " 0x";
  res += val;
  res += ", end_handle: ";
  snprintf(val, sizeof(val), "%d", m_endHandle);
  res += val;
  snprintf(val, sizeof(val), "%04x", m_endHandle);
  res += " 0x";
  res += val;

  for (auto &it : m_characteristicVector) {
    res += "\n" + it->toString();
  }

  return res;
}// toString

}

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_CENTRAL */
