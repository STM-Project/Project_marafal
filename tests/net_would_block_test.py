"""Regression test for the real adapter's F_GETFL check, with mocked fcntl."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / "MBEDTLS/App/net_sockets.c").read_text()
start = source.index("static int net_would_block( const mbedtls_net_context *ctx )\n{")
end = source.index("\n}\n", start) + 3
function = source[start:end]
with tempfile.TemporaryDirectory(prefix="stm32-net-test-") as directory:
    folder = Path(directory)
    program = r'''
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
typedef struct { int fd; } mbedtls_net_context;
static int returned_flags;
static int fake_fcntl(int fd, int cmd, int arg) {
    assert(fd == 7 && cmd == F_GETFL && arg == 0);
    errno = 0; /* LwIP F_GETFL resets errno; preserve the receive error. */
    return returned_flags;
}
#define fcntl fake_fcntl
''' + function + r'''
int main(void) {
    mbedtls_net_context net = {7};
    returned_flags = O_NONBLOCK | O_RDWR; errno = EAGAIN;
    assert(net_would_block(&net) == 1 && errno == EAGAIN);
    returned_flags = O_NONBLOCK; errno = EWOULDBLOCK;
    assert(net_would_block(&net) == 1 && errno == EWOULDBLOCK);
    returned_flags = O_RDWR; errno = EAGAIN;
    assert(net_would_block(&net) == 0 && errno == EAGAIN);
    returned_flags = -1; errno = EAGAIN;
    assert(net_would_block(&net) == 0 && errno == EAGAIN);
    returned_flags = O_NONBLOCK | O_RDWR; errno = ECONNRESET;
    assert(net_would_block(&net) == 0 && errno == ECONNRESET);
    puts("Nonblocking socket flags and errno preservation: PASS");
}
'''
    (folder / "test.c").write_text(program)
    subprocess.run(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
                    str(folder / "test.c"), "-o", str(folder / "test")], check=True)
    subprocess.run([str(folder / "test")], check=True)
