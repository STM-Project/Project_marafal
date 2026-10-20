#ifndef GOOGLE_CLOUD_CONFIG_H
#define GOOGLE_CLOUD_CONFIG_H
/* Copy the example to the ignored google_cloud_config.local.h. No Gmail password. */
#if defined(__has_include)
#if __has_include("google_cloud_config.local.h")
#include "google_cloud_config.local.h"
#endif
#endif
#ifndef GOOGLE_CLOUD_ENABLED
#define GOOGLE_CLOUD_ENABLED 1
#endif
#ifndef GOOGLE_CLOUD_DEVICE
#define GOOGLE_CLOUD_DEVICE "stm32f769-demo"
#endif
#ifndef GOOGLE_CLOUD_TOKEN
#define GOOGLE_CLOUD_TOKEN "91d483513b0a48849c922f5c67e35b4aefa7d5b743c143e2a89cfc3d8cd11c03"
#endif
#ifndef GOOGLE_CLOUD_PATH
#define GOOGLE_CLOUD_PATH "/macros/s/AKfycbwUpJU2Md_8Vqw2h_UDqqmo0VnpHt47-PdBddD9ZGdeS83A6bBtGGVBOCPGIvJ68DulDQ/exec"
#endif
#define GOOGLE_CLOUD_PERIOD_MS 10000U
#define GOOGLE_CLOUD_TIMEOUT_MS 25000U
#endif
