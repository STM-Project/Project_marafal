#include "FreeRTOS.h"
#include <stdint.h>
#include <string.h>

void *GoogleCloud_Calloc(size_t count, size_t size)
{
    if (!count || !size || count > SIZE_MAX / size) return NULL;
    size_t bytes = count * size;
    void *pointer = pvPortMalloc(bytes);
    if (pointer) memset(pointer, 0, bytes);
    return pointer;
}

void GoogleCloud_Free(void *pointer)
{
    if (pointer) vPortFree(pointer);
}
