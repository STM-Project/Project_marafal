#ifndef GOOGLE_CLOUD_LWIP_CONFIG_H
#define GOOGLE_CLOUD_LWIP_CONFIG_H
/* Project compiler preinclude: leave the generated lwipopts.h untouched. */
#include <stdint.h>
void GoogleCloud_SetUtc(uint32_t seconds);
#define SNTP_SERVER_DNS 1
#define SNTP_CHECK_RESPONSE 2
#define SNTP_SET_SYSTEM_TIME(sec) GoogleCloud_SetUtc((uint32_t)(sec))
#endif
