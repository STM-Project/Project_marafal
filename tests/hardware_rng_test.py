"""Exercise the real RNG adapter with a deterministic mock HAL, without hardware."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="stm32-rng-test-") as folder:
    tmp = Path(folder)
    (tmp / "mbedtls").mkdir()
    headers = {
        "main.h": "",
        "mbedtls_config.h": "#define MBEDTLS_ENTROPY_HARDWARE_ALT\n",
        "stm32f7xx_hal.h": """#include <stdint.h>
typedef struct { int unused; } RNG_HandleTypeDef;
#define HAL_OK 0
int HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef *, uint32_t *);
""",
        "mbedtls/entropy_poll.h": "#include <stddef.h>\n",
        "mbedtls/entropy.h": "#define MBEDTLS_ERR_ENTROPY_SOURCE_FAILED -0x003c\n",
    }
    for name, content in headers.items():
        (tmp / name).write_text(content)
    (tmp / "test.c").write_text(r'''
#include "stm32f7xx_hal.h"
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
RNG_HandleTypeDef hrng;
static int calls, failAt;
int HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef *h, uint32_t *word) {
    (void)h;
    if (++calls == failAt) return -1;
    *word = calls == 1 ? 0x44332211u : 0x88776655u;
    return HAL_OK;
}
int mbedtls_hardware_poll(void *, unsigned char *, size_t, size_t *);
int main(void) {
    unsigned char b[10]; size_t used;
    const unsigned char expected[] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
    for (size_t n=0; n<=8; ++n) {
        memset(b,0xaa,sizeof(b)); calls=0; failAt=0; used=999;
        assert(mbedtls_hardware_poll(NULL,b+1,n,&used)==0);
        assert(used==n && b[0]==0xaa && b[n+1]==0xaa);
        assert(memcmp(b+1,expected,n)==0);
    }
    calls=0; failAt=1; used=999;
    assert(mbedtls_hardware_poll(NULL,b,7,&used)==-0x003c && used==0);
    calls=0; failAt=2; used=999;
    assert(mbedtls_hardware_poll(NULL,b,7,&used)==-0x003c && used==4);
    puts("Hardware RNG copying, partial words, bounds and error propagation: PASS");
}
''')
    executable = tmp / "rng-test"
    subprocess.run([
        "cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
        "-fsanitize=address,undefined", "-I" + str(tmp),
        str(root / "MBEDTLS/Target/hardware_rng.c"), str(tmp / "test.c"),
        "-o", str(executable),
    ], check=True)
    subprocess.run([str(executable)], check=True)
