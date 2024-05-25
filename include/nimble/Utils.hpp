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
#if defined(CONFIG_BT_NIMBLE_ENABLED)

#include "host/ble_gap.h"

/****  FIX COMPILATION ****/
#undef min
#undef max
/**************************/

#include <string>

namespace nimble {

typedef struct {
  void *pATT;
  TaskHandle_t task;
  int rc;
  void *buf;
} ble_task_data_t;

/**
 * @brief A BLE Utility class with methods for debugging and general purpose use.
 */
class Utils {
public:
  static void dumpGapEvent(ble_gap_event *event, void *arg);
  static const char *gapEventToString(uint8_t eventType);
  static char *buildHexData(uint8_t *target, const uint8_t *source, uint8_t length);
  static const char *advTypeToString(uint8_t advType);
  static const char *returnCodeToString(int rc);
  static int checkConnParams(ble_gap_conn_params *params);
};

}

#endif // CONFIG_BT_ENABLED
