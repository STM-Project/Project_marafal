/* Application implementation of mbedTLS's supported platform time hook.
 * Unlike gmtime(), gmtime_r() uses the caller's result buffer and does not
 * need newlib to allocate its internal struct tm on the C library heap.
 */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include "mbedtls/platform_util.h"
#include <time.h>

#if defined(MBEDTLS_HAVE_TIME_DATE) && defined(MBEDTLS_PLATFORM_GMTIME_R_ALT)
struct tm *mbedtls_platform_gmtime_r(const mbedtls_time_t *timestamp,
                                    struct tm *result)
{
    if (timestamp == NULL || result == NULL) return NULL;
    return gmtime_r(timestamp, result);
}
#endif
