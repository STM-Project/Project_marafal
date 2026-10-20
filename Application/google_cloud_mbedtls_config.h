#ifndef GOOGLE_CLOUD_MBEDTLS_CONFIG_H
#define GOOGLE_CLOUD_MBEDTLS_CONFIG_H
/* Wrap the original configuration without editing MBEDTLS or library sources. */
#include "mbedtls_config.h"
#include <stddef.h>

#define MBEDTLS_PLATFORM_TIME_ALT
#define MBEDTLS_PLATFORM_GMTIME_R_ALT

/* All TLS users share the application SDRAM allocator. The original server's optional
 * private buffer allocator is disabled via configuration, not source edits.
 */
#undef MBEDTLS_MEMORY_BUFFER_ALLOC_C
#include "google_cloud_memory.h"
#define MBEDTLS_PLATFORM_CALLOC_MACRO GoogleCloud_Calloc
#define MBEDTLS_PLATFORM_FREE_MACRO GoogleCloud_Free
#endif
