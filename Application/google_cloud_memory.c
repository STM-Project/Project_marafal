#include "FreeRTOS.h"
#include "semphr.h"
#include "google_cloud_memory.h"
#include <stdint.h>
#include <string.h>

/* Dedicated TLS arena, independent of the 128 KiB task heap.
 * Allocation units are 512 bytes; adjacent free units can serve any request.
 * Fixed metadata and a static mutex avoid allocating allocator bookkeeping.
 */
#define UNIT_SIZE 512U
#define UNIT_COUNT 1024U
#define CONTINUATION UINT16_MAX
static unsigned char tlsPool[UNIT_SIZE * UNIT_COUNT]
    __attribute__((section(".sdram"), aligned(32)));
static uint16_t allocations[UNIT_COUNT];
static StaticSemaphore_t mutexStorage;
static SemaphoreHandle_t mutex;
volatile GoogleCloudMemoryStatus googleCloudMemoryStatus;

static void updateStatus(void)
{
    size_t freeUnits = 0, run = 0, longest = 0;
    for (size_t i = 0; i < UNIT_COUNT; ++i) {
        if (!allocations[i]) {
            ++freeUnits;
            if (++run > longest) longest = run;
        } else run = 0;
    }
    googleCloudMemoryStatus.free_bytes = freeUnits * UNIT_SIZE;
    googleCloudMemoryStatus.largest_free_block = longest * UNIT_SIZE;
}

int GoogleCloud_MemoryInit(void)
{
    if (mutex) return 0; /* Never reset a pool already holding TLS contexts. */
    memset(allocations, 0, sizeof(allocations));
    mutex = xSemaphoreCreateMutexStatic(&mutexStorage);
    if (!mutex) return -1;
    updateStatus();
    googleCloudMemoryStatus.ready = 1;
    return 0;
}

void *GoogleCloud_Calloc(size_t count, size_t size)
{
    if (!count || !size || !mutex) return NULL;
    if (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) return NULL;
    if (count > SIZE_MAX / size) {
        googleCloudMemoryStatus.last_request = SIZE_MAX;
        ++googleCloudMemoryStatus.failures;
        xSemaphoreGive(mutex);
        return NULL;
    }
    size_t bytes = count * size;
    size_t needed = bytes <= sizeof(tlsPool) ? (bytes + UNIT_SIZE - 1) / UNIT_SIZE : UNIT_COUNT + 1;
    size_t run = 0, start = 0;
    void *pointer = NULL;
    googleCloudMemoryStatus.last_request = bytes;
    for (size_t i = 0; i < UNIT_COUNT && needed <= UNIT_COUNT; ++i) {
        if (!allocations[i]) ++run; else run = 0;
        if (run == needed) {
            start = i + 1 - needed;
            allocations[start] = (uint16_t)needed;
            for (size_t j = start + 1; j <= i; ++j) allocations[j] = CONTINUATION;
            pointer = tlsPool + start * UNIT_SIZE;
            break;
        }
    }
    if (!pointer) ++googleCloudMemoryStatus.failures;
    updateStatus();
    xSemaphoreGive(mutex);
    if (pointer) memset(pointer, 0, bytes);
    return pointer;
}

void GoogleCloud_Free(void *pointer)
{
    if (!pointer || !mutex) return;
    if (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) return;
    uintptr_t address = (uintptr_t)pointer, base = (uintptr_t)tlsPool;
    if (address < base || address >= base + sizeof(tlsPool) || (address-base) % UNIT_SIZE) {
        ++googleCloudMemoryStatus.invalid_frees;
    } else {
        size_t index = (address-base) / UNIT_SIZE;
        size_t count = allocations[index];
        if (!count || count == CONTINUATION || index+count > UNIT_COUNT) {
            ++googleCloudMemoryStatus.invalid_frees;
        } else {
            memset(allocations+index, 0, count*sizeof(allocations[0]));
            updateStatus();
        }
    }
    xSemaphoreGive(mutex);
}
