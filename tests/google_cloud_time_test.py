"""Exercise the application UTC hook against known UTC dates and buffer reuse."""
import calendar
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
cases = [(1970, 1, 1, 0, 0, 0), (1970, 1, 1, 0, 0, 1),
         (2000, 2, 29, 12, 30, 15), (2024, 2, 29, 23, 59, 59),
         (2026, 10, 20, 3, 49, 0), (2027, 1, 1, 0, 0, 0),
         (2036, 6, 22, 0, 0, 0)]
checks = []
for year, month, day, hour, minute, second in cases:
    epoch = calendar.timegm((year, month, day, hour, minute, second))
    checks.append(f'''timestamp = {epoch};
    assert(mbedtls_platform_gmtime_r(&timestamp, &a) == &a);
    assert(a.tm_year == {year-1900} && a.tm_mon == {month-1} && a.tm_mday == {day});
    assert(a.tm_hour == {hour} && a.tm_min == {minute} && a.tm_sec == {second});
    assert(a.tm_isdst == 0);''')
with tempfile.TemporaryDirectory(prefix="stm32-time-test-") as directory:
    folder = Path(directory)
    (folder / "mbedtls").mkdir()
    (folder / "mbedtls/platform_util.h").write_text('''#include <time.h>
#define MBEDTLS_HAVE_TIME_DATE
#define MBEDTLS_PLATFORM_GMTIME_R_ALT
typedef time_t mbedtls_time_t;
struct tm *mbedtls_platform_gmtime_r(const mbedtls_time_t *, struct tm *);
''')
    (folder / "test.c").write_text('''#include "mbedtls/platform_util.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    mbedtls_time_t timestamp = 0;
    struct tm a, b;
    assert(mbedtls_platform_gmtime_r(NULL, &a) == NULL);
    assert(mbedtls_platform_gmtime_r(&timestamp, NULL) == NULL);
''' + "\n".join(checks) + '''
    timestamp = 0;
    assert(mbedtls_platform_gmtime_r(&timestamp, &b) == &b);
    assert(a.tm_year == 136 && b.tm_year == 70);
    puts("Application TLS UTC hook: epochs, leap days, UTC and independent buffers: PASS");
}
''')
    subprocess.run(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=address,undefined", "-I" + str(folder),
                    str(root / "Application/google_cloud_time.c"),
                    str(folder / "test.c"), "-o", str(folder / "test")], check=True)
    subprocess.run([str(folder / "test")], check=True)
