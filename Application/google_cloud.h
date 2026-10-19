#ifndef GOOGLE_CLOUD_H
#define GOOGLE_CLOUD_H
#include <stdint.h>

typedef struct {
    uint32_t successes;
    uint32_t failures;
    int last_error;             /* 0=OK, -1=config, -2=clock, others=network/TLS/protocol */
    int temperature10;          /* 255 = 25.5 Celsius */
    int demo_relay;             /* RAM only; no GPIO writes */
    uint32_t certificate_flags;
} GoogleCloudStatus;
extern volatile GoogleCloudStatus googleCloudStatus;
void GoogleCloud_Start(void);
/* Called by LwIP SNTP, in UTC, not local time. */
void GoogleCloud_SetUtc(uint32_t seconds);
/* Override when an actual sensor is available; default returns 255. */
int GoogleCloud_ReadTemperature10(void);
#endif
