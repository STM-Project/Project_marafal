#ifndef GOOGLE_CLOUD_CONFIG_H
#define GOOGLE_CLOUD_CONFIG_H
/* Copy the example to the ignored google_cloud_config.local.h. No Gmail password. */
#if defined(__has_include)
#if __has_include("google_cloud_config.local.h")
#include "google_cloud_config.local.h"
#endif
#endif
#ifndef GOOGLE_CLOUD_ENABLED
#define GOOGLE_CLOUD_ENABLED 0
#endif
#ifndef GOOGLE_CLOUD_DEVICE
#define GOOGLE_CLOUD_DEVICE "stm32f769-demo"
#endif
#ifndef GOOGLE_CLOUD_TOKEN
#define GOOGLE_CLOUD_TOKEN ""
#endif
#ifndef GOOGLE_CLOUD_PATH
#define GOOGLE_CLOUD_PATH ""
#endif
#define GOOGLE_CLOUD_PERIOD_MS 60000U
#define GOOGLE_CLOUD_TIMEOUT_MS 25000U
#endif
