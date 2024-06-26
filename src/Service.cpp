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

#include "nimble/Device.hpp"
#include "nimble/Log.hpp"
#include "nimble/Service.hpp"
#include "nimble/Utils.hpp"

#include <string>

namespace nimble {

static const char *LOG_TAG = "NimBLEService";// Tag for logging.

#define NULL_HANDLE (0xffff)

/**
 * @brief Construct an instance of the NimBLEService
 * @param [in] uuid The UUID of the service.
 */
Service::Service(char const *uuid) : Service(UUID(uuid)) {
  (void) uuid;
}

/**
 * @brief Construct an instance of the BLEService
 * @param [in] uuid The UUID of the service.
 */
Service::Service(const UUID &uuid) {
  m_uuid = uuid;
  m_handle = NULL_HANDLE;
  m_pSvcDef = nullptr;
  m_removed = 0;

}// NimBLEService

Service::~Service() {
  if (m_pSvcDef != nullptr) {
    if (m_pSvcDef->characteristics != nullptr) {
      for (int i = 0; m_pSvcDef->characteristics[i].uuid != nullptr; ++i) {
        delete (m_pSvcDef->characteristics[i].descriptors);
      }
      delete (m_pSvcDef->characteristics);
    }

    delete (m_pSvcDef);
  }

  for (auto const &characteristic : m_characteristics) {
    delete characteristic;
  }
}

/**
 * @brief Dump details of this BLE GATT service.
 */
void Service::dump() {
  NIMBLE_LOGD(LOG_TAG, "Service: uuid:%s, handle: 0x%2x", m_uuid.toString().c_str(), m_handle);

  std::string res;
  int count = 0;
  char hex[5];
  for (auto const &characteristic : m_characteristics) {
    if (count > 0) {
      res += "\n";
    }
    snprintf(hex, sizeof(hex), "%04x", characteristic->getHandle());
    count++;
    res += "handle: 0x";
    res += hex;
    res += ", uuid: " + std::string(characteristic->getUUID());
  }
  NIMBLE_LOGD(LOG_TAG, "Characteristics:\n%s", res.c_str());
}// dump

Server *Service::getServer() {
  return Device::getServer();
}

/**
 * @brief Get the UUID of the service.
 * @return the UUID of the service.
 */
UUID Service::getUUID() {
  return m_uuid;
}// getUUID

/**
 * @brief Builds the database of characteristics/descriptors for the service
 * and registers it with the NimBLE stack.
 * @return bool success/failure .
 */
bool Service::start() {
  NIMBLE_LOGD(LOG_TAG, ">> start(): Starting service: %s", toString().c_str());

  // Rebuild the service definition if the server attributes have changed.
  if (getServer()->m_svcChanged && m_pSvcDef != nullptr) {
    if (m_pSvcDef[0].characteristics) {
      delete (m_pSvcDef[0].characteristics[0].descriptors);
      delete (m_pSvcDef[0].characteristics);
    }
    delete (m_pSvcDef);
    m_pSvcDef = nullptr;
  }

  if (m_pSvcDef == nullptr) {
    // Nimble requires an array of services to be sent to the api
    // Since we are adding 1 at a time we create an array of 2 and set the type
    // of the second service to 0 to indicate the end of the array.
    auto svc = new ble_gatt_svc_def[2];
    ble_gatt_chr_def *pChr_a;
    ble_gatt_dsc_def *pDsc_a;

    svc[0].type = BLE_GATT_SVC_TYPE_PRIMARY;
    svc[0].uuid = &m_uuid.getNative()->u;
    svc[0].includes = nullptr;

    int removedCount = 0;
    for (auto it = m_characteristics.begin(); it != m_characteristics.end();) {
      if ((*it)->m_removed > 0) {
        if ((*it)->m_removed == NIMBLE_ATT_REMOVE_DELETE) {
          delete *it;
          it = m_characteristics.erase(it);
        } else {
          ++removedCount;
          ++it;
        }
        continue;
      }

      ++it;
    }

    size_t numChrs = m_characteristics.size() - removedCount;
    NIMBLE_LOGD(LOG_TAG, "Adding %d characteristics for service %s", numChrs, toString().c_str());

    if (!numChrs) {
      svc[0].characteristics = nullptr;
    } else {
      // Nimble requires the last characteristic to have it's uuid = 0 to indicate the end
      // of the characteristics for the service. We create 1 extra and set it to null
      // for this purpose.
      pChr_a = new ble_gatt_chr_def[numChrs + 1]{};
      int i = 0;
      for (auto &m_characteristic : m_characteristics) {
        if (m_characteristic->m_removed > 0) {
          continue;
        }

        removedCount = 0;
        for (auto it = m_characteristic->m_dscVec.begin(); it != m_characteristic->m_dscVec.end();) {
          if ((*it)->m_removed > 0) {
            if ((*it)->m_removed == NIMBLE_ATT_REMOVE_DELETE) {
              delete *it;
              it = m_characteristic->m_dscVec.erase(it);
            } else {
              ++removedCount;
              ++it;
            }
            continue;
          }

          ++it;
        }

        size_t numDscs = m_characteristic->m_dscVec.size() - removedCount;

        if (!numDscs) {
          pChr_a[i].descriptors = nullptr;
        } else {
          // Must have last descriptor uuid = 0 so we have to create 1 extra
          pDsc_a = new ble_gatt_dsc_def[numDscs + 1];
          int d = 0;
          for (auto &dsc_it : m_characteristic->m_dscVec) {
            if (dsc_it->m_removed > 0) {
              continue;
            }
            pDsc_a[d].uuid = &dsc_it->m_uuid.getNative()->u;
            pDsc_a[d].att_flags = dsc_it->m_properties;
            pDsc_a[d].min_key_size = 0;
            pDsc_a[d].access_cb = Descriptor::handleGapEvent;
            pDsc_a[d].arg = dsc_it;
            ++d;
          }

          pDsc_a[numDscs].uuid = nullptr;
          pChr_a[i].descriptors = pDsc_a;
        }

        pChr_a[i].uuid = &m_characteristic->m_uuid.getNative()->u;
        pChr_a[i].access_cb = Characteristic::handleGapEvent;
        pChr_a[i].arg = m_characteristic;
        pChr_a[i].flags = m_characteristic->m_properties;
        pChr_a[i].min_key_size = 0;
        pChr_a[i].val_handle = &m_characteristic->m_handle;
        ++i;
      }

      pChr_a[numChrs].uuid = nullptr;
      svc[0].characteristics = pChr_a;
    }

    // end of services must indicate to api with type = 0
    svc[1].type = 0;
    m_pSvcDef = svc;
  }

  int rc = ble_gatts_count_cfg((const ble_gatt_svc_def *) m_pSvcDef);
  if (rc != 0) {
    NIMBLE_LOGE(LOG_TAG, "ble_gatts_count_cfg failed, rc= %d, %s", rc, Utils::returnCodeToString(rc));
    return false;
  }

  rc = ble_gatts_add_svcs((const ble_gatt_svc_def *) m_pSvcDef);
  if (rc != 0) {
    NIMBLE_LOGE(LOG_TAG, "ble_gatts_add_svcs, rc= %d, %s", rc, Utils::returnCodeToString(rc));
    return false;
  }

  NIMBLE_LOGD(LOG_TAG, "<< start()");
  return true;
}// start

/**
 * @brief Get the handle associated with this service.
 * @return The handle associated with this service.
 */
uint16_t Service::getHandle() {
  if (m_handle == NULL_HANDLE) {
    ble_gatts_find_svc(&getUUID().getNative()->u, &m_handle);
  }
  return m_handle;
}// getHandle

/**
 * @brief Create a new BLE Characteristic associated with this service.
 * @param [in] uuid - The UUID of the characteristic.
 * @param [in] properties - The properties of the characteristic.
 * @param [in] max_len - The maximum length in bytes that the characteristic value can hold.
 * @return The new BLE characteristic.
 */
Characteristic *Service::createCharacteristic(const char *uuid, uint32_t properties, uint16_t max_len) {
  return createCharacteristic(UUID(uuid), properties, max_len);
}

/**
 * @brief Create a new BLE Characteristic associated with this service.
 * @param [in] uuid - The UUID of the characteristic.
 * @param [in] properties - The properties of the characteristic.
 * @param [in] max_len - The maximum length in bytes that the characteristic value can hold.
 * @return The new BLE characteristic.
 */
Characteristic *Service::createCharacteristic(const UUID &uuid, uint32_t properties, uint16_t max_len) {
  auto characteristic = new Characteristic(uuid, properties, max_len, this);

  if (getCharacteristic(uuid) != nullptr) {
    NIMBLE_LOGD(LOG_TAG, "<< Adding a duplicate characteristic with UUID: %s", std::string(uuid).c_str());
  }

  addCharacteristic(characteristic);
  return characteristic;
}// createCharacteristic

CharacteristicPtr Service::createCharacteristic(uint16_t uuid, uint32_t properties, uint16_t max_len) {
  return createCharacteristic(UUID(uuid), properties, max_len);
}

/**
 * @brief Add a characteristic to the service.
 * @param[in] pCharacteristic A pointer to the characteristic instance to add to the service.
 */
void Service::addCharacteristic(Characteristic *characteristic) {
  bool foundRemoved = false;

  if (characteristic->m_removed > 0) {
    for (auto const &characteristicInMemory : m_characteristics) {
      if (characteristic == characteristicInMemory) {
        foundRemoved = true;
        characteristic->m_removed = 0;
      }
    }
  }

  if (not foundRemoved) {
    m_characteristics.push_back(characteristic);
  }

  characteristic->setService(this);
  getServer()->serviceChanged();
}// addCharacteristic

/**
 * @brief Remove a characteristic from the service.
 * @param[in] pCharacteristic A pointer to the characteristic instance to remove from the service.
 * @param[in] deleteChr If true it will delete the characteristic instance and free it's resources.
 */
void Service::removeCharacteristic(Characteristic *pCharacteristic, bool deleteChr) {
  // Check if the characteristic was already removed and if so, check if this
  // is being called to delete the object and do so if requested.
  // Otherwise, ignore the call and return.
  if (pCharacteristic->m_removed > 0) {
    if (deleteChr) {
      for (auto it = m_characteristics.begin(); it != m_characteristics.end(); ++it) {
        if ((*it) == pCharacteristic) {
          m_characteristics.erase(it);
          delete *it;
          break;
        }
      }
    }

    return;
  }

  pCharacteristic->m_removed = deleteChr ? NIMBLE_ATT_REMOVE_DELETE : NIMBLE_ATT_REMOVE_HIDE;
  getServer()->serviceChanged();
}// removeCharacteristic

/**
 * @brief Get a pointer to the characteristic object with the specified UUID.
 * @param [in] uuid The UUID of the characteristic.
 * @param instanceId The index of the characteristic to return (used when multiple characteristics have the same UUID).
 * @return A pointer to the characteristic object or nullptr if not found.
 */
Characteristic *Service::getCharacteristic(const char *uuid, uint16_t instanceId) {
  return getCharacteristic(UUID(uuid), instanceId);
}

/**
 * @brief Get a pointer to the characteristic object with the specified UUID.
 * @param [in] uuid The UUID of the characteristic.
 * @param instanceId The index of the characteristic to return (used when multiple characteristics have the same UUID).
 * @return A pointer to the characteristic object or nullptr if not found.
 */
Characteristic *Service::getCharacteristic(const UUID &uuid, uint16_t instanceId) {
  uint16_t position = 0;
  for (auto const &characteristic : m_characteristics) {
    if (characteristic->getUUID() == uuid) {
      if (position == instanceId) {
        return characteristic;
      }
      position++;
    }
  }
  return nullptr;
}

/**
 * @brief Get a pointer to the characteristic object with the specified handle.
 * @param handle The handle of the characteristic.
 * @return A pointer to the characteristic object or nullptr if not found.
 */
Characteristic *Service::getCharacteristicByHandle(uint16_t handle) {
  for (auto const &characteristic : m_characteristics) {
    if (characteristic->getHandle() == handle) {
      return characteristic;
    }
  }
  return nullptr;
}

/**
 * @return A vector containing pointers to each characteristic associated with this service.
 */
std::vector<Characteristic *> Service::getCharacteristics() {
  return m_characteristics;
}

/**
 * @return A vector containing pointers to each characteristic with the provided UUID associated with this service.
 */
std::vector<Characteristic *> Service::getCharacteristics(const char *uuid) {
  return getCharacteristics(UUID(uuid));
}

/**
 * @return A vector containing pointers to each characteristic with the provided UUID associated with this service.
 */
std::vector<Characteristic *> Service::getCharacteristics(const UUID &uuid) {
  std::vector<Characteristic *> result;
  for (auto const &characteristic : m_characteristics) {
    if (characteristic->getUUID() == uuid) {
      result.push_back(characteristic);
    }
  }
  return result;
}

/**
 * @brief Return a string representation of this service.
 * A service is defined by:
 * * Its UUID
 * * Its handle
 * @return A string representation of this service.
 */
std::string Service::toString() {
  std::string res = "UUID: " + getUUID().toString();
  char hex[5];
  snprintf(hex, sizeof(hex), "%04x", getHandle());
  res += ", handle: 0x";
  res += hex;
  return res;
}

// toString

}// namespace nimble

#endif /* CONFIG_BT_ENABLED && CONFIG_BT_NIMBLE_ROLE_PERIPHERAL */
