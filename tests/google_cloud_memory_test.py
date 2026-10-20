"""Test the real TLS arena: exhaustion, fragmentation, zeroing and concurrency."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="stm32-memory-test-") as directory:
    folder = Path(directory)
    (folder / "FreeRTOS.h").write_text('''#include <stddef.h>
#include <stdint.h>
#define pdTRUE 1
#define portMAX_DELAY UINT32_MAX
''')
    (folder / "semphr.h").write_text('''#include <pthread.h>
typedef pthread_mutex_t StaticSemaphore_t;
typedef pthread_mutex_t *SemaphoreHandle_t;
static inline SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *p) {
    return pthread_mutex_init(p, NULL) == 0 ? p : NULL;
}
static inline int xSemaphoreTake(SemaphoreHandle_t p, unsigned timeout) {
    (void)timeout; return pthread_mutex_lock(p) == 0;
}
static inline void xSemaphoreGive(SemaphoreHandle_t p) { pthread_mutex_unlock(p); }
''')
    (folder / "test.c").write_text(r'''
#include "google_cloud_memory.h"
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#define CAPACITY (512u*1024u)
static void checkZero(const unsigned char *p, size_t n) {
    assert(p); for(size_t i=0;i<n;i++) assert(p[i]==0);
}
static void *worker(void *argument) {
    uintptr_t id=(uintptr_t)argument;
    for(size_t i=0;i<200;i++) {
        size_t n=1+(i*977+id*193)%16384;
        unsigned char *p=GoogleCloud_Calloc(1,n);
        checkZero(p,n); memset(p,(int)id,n); sched_yield();
        for(size_t j=0;j<n;j++) assert(p[j]==id);
        GoogleCloud_Free(p);
    }
    return NULL;
}
int main(void) {
    assert(GoogleCloud_Calloc(1,1)==NULL);
    assert(GoogleCloud_MemoryInit()==0 && googleCloudMemoryStatus.ready);
    assert(googleCloudMemoryStatus.free_bytes==CAPACITY);
    unsigned char *a=GoogleCloud_Calloc(1,CAPACITY);
    checkZero(a,CAPACITY); assert((uintptr_t)a%32==0);
    assert(GoogleCloud_Calloc(1,1)==NULL);
    assert(GoogleCloud_MemoryInit()==0); /* must not reset live allocations */
    assert(googleCloudMemoryStatus.free_bytes==0);
    GoogleCloud_Free(a+1); GoogleCloud_Free(a+512);
    assert(googleCloudMemoryStatus.invalid_frees==2);
    memset(a,0xa5,CAPACITY); GoogleCloud_Free(a);
    GoogleCloud_Free(a); assert(googleCloudMemoryStatus.invalid_frees==3);
    a=GoogleCloud_Calloc(2,CAPACITY/2); checkZero(a,CAPACITY); GoogleCloud_Free(a);
    assert(GoogleCloud_Calloc(SIZE_MAX,2)==NULL);
    assert(googleCloudMemoryStatus.last_request==SIZE_MAX);
    assert(GoogleCloud_Calloc(1,CAPACITY+1)==NULL);
    assert(GoogleCloud_Calloc(0,10)==NULL);
    a=GoogleCloud_Calloc(1,CAPACITY/4);
    void *b=GoogleCloud_Calloc(1,CAPACITY/4);
    void *c=GoogleCloud_Calloc(1,CAPACITY/4);
    void *d=GoogleCloud_Calloc(1,CAPACITY/4);
    assert(a && b && c && d);
    GoogleCloud_Free(a); GoogleCloud_Free(c);
    assert(googleCloudMemoryStatus.free_bytes==CAPACITY/2);
    assert(googleCloudMemoryStatus.largest_free_block==CAPACITY/4);
    assert(GoogleCloud_Calloc(1,CAPACITY/2)==NULL);
    GoogleCloud_Free(b);
    void *big=GoogleCloud_Calloc(1,300*1024); assert(big);
    GoogleCloud_Free(big); GoogleCloud_Free(d);
    pthread_t threads[4];
    for(uintptr_t i=0;i<4;i++) assert(pthread_create(&threads[i],NULL,worker,(void *)(i+1))==0);
    for(size_t i=0;i<4;i++) assert(pthread_join(threads[i],NULL)==0);
    assert(googleCloudMemoryStatus.free_bytes==CAPACITY);
    assert(googleCloudMemoryStatus.largest_free_block==CAPACITY);
    puts("TLS SDRAM arena: exhaustion, overflow, reuse, fragmentation, alignment and concurrent clients: PASS");
}
''')
    subprocess.run(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-pthread",
                    "-fsanitize=address,undefined", "-I" + str(folder),
                    "-I" + str(root / "Application"),
                    str(root / "Application/google_cloud_memory.c"),
                    str(folder / "test.c"), "-o", str(folder / "test")], check=True)
    subprocess.run([str(folder / "test")], check=True)
