"""Test the application BIO callbacks with mocked nonblocking LwIP sockets."""
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='stm32-transport-test-') as directory:
    folder = Path(directory)
    (folder/'lwip').mkdir()
    (folder/'mbedtls').mkdir()
    (folder/'lwip/sockets.h').write_text('''#include <stddef.h>
int lwip_recv(int, void *, size_t, int);
int lwip_send(int, const void *, size_t, int);
''')
    (folder/'mbedtls/net_sockets.h').write_text('''typedef struct {int fd;} mbedtls_net_context;
#define MBEDTLS_ERR_NET_INVALID_CONTEXT -1
#define MBEDTLS_ERR_NET_CONN_RESET -2
#define MBEDTLS_ERR_NET_RECV_FAILED -3
#define MBEDTLS_ERR_NET_SEND_FAILED -4
''')
    (folder/'mbedtls/ssl.h').write_text('''#define MBEDTLS_ERR_SSL_WANT_READ -5
#define MBEDTLS_ERR_SSL_WANT_WRITE -6
''')
    (folder/'test.c').write_text(r'''
#include "google_cloud_transport.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include <errno.h>
#include <assert.h>
#include <stdio.h>
static int result, error;
int lwip_recv(int fd, void *data, size_t len, int flags) {
    assert(fd==7 && data && len==8 && flags==0);
    errno=error; return result;
}
int lwip_send(int fd, const void *data, size_t len, int flags) {
    assert(fd==7 && data && len==8 && flags==0);
    errno=error; return result;
}
int main(void) {
    mbedtls_net_context net={7}, invalid={-1}; unsigned char data[8];
    assert(GoogleCloud_TlsRecv(NULL,data,8)==MBEDTLS_ERR_NET_INVALID_CONTEXT);
    assert(GoogleCloud_TlsSend(&invalid,data,8)==MBEDTLS_ERR_NET_INVALID_CONTEXT);
    result=-1;
    int retry[]={EAGAIN,EWOULDBLOCK,EINTR};
    for(unsigned i=0;i<sizeof(retry)/sizeof(retry[0]);i++) {
        error=retry[i];
        assert(GoogleCloud_TlsRecv(&net,data,8)==MBEDTLS_ERR_SSL_WANT_READ);
        assert(GoogleCloud_TlsSend(&net,data,8)==MBEDTLS_ERR_SSL_WANT_WRITE);
    }
    error=ECONNRESET;
    assert(GoogleCloud_TlsRecv(&net,data,8)==MBEDTLS_ERR_NET_CONN_RESET);
    error=EPIPE;
    assert(GoogleCloud_TlsSend(&net,data,8)==MBEDTLS_ERR_NET_CONN_RESET);
    error=EBADF;
    assert(GoogleCloud_TlsRecv(&net,data,8)==MBEDTLS_ERR_NET_RECV_FAILED);
    assert(GoogleCloud_TlsSend(&net,data,8)==MBEDTLS_ERR_NET_SEND_FAILED);
    for(int n=0;n<=8;n++) {
        result=n;
        assert(GoogleCloud_TlsRecv(&net,data,8)==n);
        assert(GoogleCloud_TlsSend(&net,data,8)==n);
    }
    puts("Application TLS transport: nonblocking retry, partial I/O, EOF and errors: PASS");
}
''')
    subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',
                    '-fsanitize=address,undefined','-I'+str(folder),
                    '-I'+str(root/'Application'),
                    str(root/'Application/google_cloud_transport.c'),
                    str(folder/'test.c'),'-o',str(folder/'test')],check=True)
    subprocess.run([str(folder/'test')],check=True)
