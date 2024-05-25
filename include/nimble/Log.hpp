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

#include <esp_log.h>
#ifndef CONFIG_NIMBLE_CPP_LOG_LEVEL
#define CONFIG_NIMBLE_CPP_LOG_LEVEL 0
#endif

#define NIMBLE_CPP_LOG_PRINT(level, tag, format, ...)         \
  do {                                                        \
    if (CONFIG_NIMBLE_CPP_LOG_LEVEL >= level)                 \
      ESP_LOG_LEVEL_LOCAL(level, tag, format, ##__VA_ARGS__); \
  } while (0)

#define NIMBLE_LOGD(tag, format, ...) \
  NIMBLE_CPP_LOG_PRINT(ESP_LOG_DEBUG, tag, format, ##__VA_ARGS__)

#define NIMBLE_LOGI(tag, format, ...) \
  NIMBLE_CPP_LOG_PRINT(ESP_LOG_INFO, tag, format, ##__VA_ARGS__)

#define NIMBLE_LOGW(tag, format, ...) \
  NIMBLE_CPP_LOG_PRINT(ESP_LOG_WARN, tag, format, ##__VA_ARGS__)

#define NIMBLE_LOGE(tag, format, ...) \
  NIMBLE_CPP_LOG_PRINT(ESP_LOG_ERROR, tag, format, ##__VA_ARGS__)

#define NIMBLE_LOGC(tag, format, ...) \
  NIMBLE_CPP_LOG_PRINT(ESP_LOG_ERROR, tag, format, ##__VA_ARGS__)

#endif /* CONFIG_BT_ENABLED */
