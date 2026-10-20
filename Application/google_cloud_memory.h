#ifndef GOOGLE_CLOUD_MEMORY_H
#define GOOGLE_CLOUD_MEMORY_H
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t ready;
    size_t free_bytes;
    size_t largest_free_block;
    size_t last_request;
    uint32_t failures;
    uint32_t invalid_frees;
} GoogleCloudMemoryStatus;
extern volatile GoogleCloudMemoryStatus googleCloudMemoryStatus;

/* Call once after SDRAM initialization and before starting any TLS tasks. */
int GoogleCloud_MemoryInit(void);
void *GoogleCloud_Calloc(size_t count, size_t size);
void GoogleCloud_Free(void *pointer);
#endif
