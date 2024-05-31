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
#if defined(CONFIG_BT_NIMBLE_ENABLED) && defined(CONFIG_BT_NIMBLE_ROLE_OBSERVER)

#include "nimble/AdvertisedDevice.hpp"
#include "nimble/Utils.hpp"

#include "host/ble_gap.h"

#include <vector>

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

namespace nimble {

class Device;
class Scan;
class AdvertisedDevice;
class ScanCallbacks;
class Address;

/**
 * @brief A class that contains and operates on the results of a BLE scan.
 * @details When a scan completes, we have a set of found devices.  Each device is described
 * by a NimBLEAdvertisedDevice object.  The number of items in the set is given by
 * getCount().  We can retrieve a device by calling getDevice() passing in the
 * index (starting at 0) of the desired device.
 */
class ScanResults {
  friend class Scan;

public:
  void dump();
  std::size_t getCount();
  AdvertisedDevice getDevice(uint32_t i);
  std::vector<AdvertisedDevice *>::iterator begin();
  std::vector<AdvertisedDevice *>::iterator end();
  AdvertisedDevice *getDevice(const Address &address);

private:
  std::vector<AdvertisedDevice *> m_advertisedDevicesVector;
};

/**
 * @brief Perform and manage %BLE scans.
 *
 * Scanning is associated with a %BLE client that is attempting to locate BLE servers.
 */
class Scan {
  friend class Device;

public:
  bool start(uint32_t duration, bool is_continue = false);
  bool isScanning();
  void setScanCallbacks(ScanCallbacks *pScanCallbacks, bool wantDuplicates = false);
  void setActiveScan(bool active);
  void setInterval(uint16_t intervalMSecs);
  void setWindow(uint16_t windowMSecs);
  void setDuplicateFilter(bool enabled);
  void setLimitedOnly(bool enabled);
  void setFilterPolicy(uint8_t filter);
  void clearDuplicateCache();
  bool stop();
  void clearResults();
  ScanResults getResults();
  ScanResults getResults(uint32_t duration, bool is_continue = false);
  void setMaxResults(uint8_t maxResults);
  void erase(const Address &address);

private:
  Scan();
  ~Scan();

private:
  static int handleGapEvent(ble_gap_event *event, void *arg);
  void onHostReset();
  void onHostSync();

private:
  ScanCallbacks *m_pScanCallbacks;
  ble_gap_disc_params m_scan_params{};
  bool m_ignoreResults;
  ScanResults m_scanResults;
  uint32_t m_duration;
  ble_task_data_t *m_pTaskData;
  uint8_t m_maxResults;
};

/**
 * @brief A callback handler for callbacks associated device scanning.
 */
class ScanCallbacks {
public:
  virtual ~ScanCallbacks() = default;

  /**
     * @brief Called when a new device is discovered, before the scan result is received (if applicable).
     * @param [in] advertisedDevice The device which was discovered.
     */
  virtual void onDiscovered(AdvertisedDevice *advertisedDevice) {};

  /**
     * @brief Called when a new scan result is complete, including scan response data (if applicable).
     * @param [in] advertisedDevice The device for which the complete result is available.
     */
  virtual void onResult(AdvertisedDevice *advertisedDevice) {};

  /**
     * @brief Called when a scan operation ends.
     * @param [in] scanResults The results of the scan that ended.
     */
  virtual void onScanEnd(ScanResults const &scanResults) {};
};

}// namespace nimble

#endif /* CONFIG_BT_ENABLED CONFIG_BT_NIMBLE_ROLE_OBSERVER */
