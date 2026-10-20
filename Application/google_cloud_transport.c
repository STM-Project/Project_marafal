#include "google_cloud_transport.h"
#include "lwip/sockets.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include <errno.h>

/* connectHost() explicitly sets O_NONBLOCK. No library flag detection is used. */
int GoogleCloud_TlsRecv(void *context, unsigned char *buffer, size_t length)
{
    mbedtls_net_context *net = context;
    if (!net || net->fd < 0) return MBEDTLS_ERR_NET_INVALID_CONTEXT;
    int result = lwip_recv(net->fd, buffer, length, 0);
    if (result >= 0) return result;
    int error = errno;
    if (error == EAGAIN || error == EWOULDBLOCK || error == EINTR)
        return MBEDTLS_ERR_SSL_WANT_READ;
    if (error == ECONNRESET || error == EPIPE) return MBEDTLS_ERR_NET_CONN_RESET;
    return MBEDTLS_ERR_NET_RECV_FAILED;
}

int GoogleCloud_TlsSend(void *context, const unsigned char *buffer, size_t length)
{
    mbedtls_net_context *net = context;
    if (!net || net->fd < 0) return MBEDTLS_ERR_NET_INVALID_CONTEXT;
    int result = lwip_send(net->fd, buffer, length, 0);
    if (result >= 0) return result;
    int error = errno;
    if (error == EAGAIN || error == EWOULDBLOCK || error == EINTR)
        return MBEDTLS_ERR_SSL_WANT_WRITE;
    if (error == ECONNRESET || error == EPIPE) return MBEDTLS_ERR_NET_CONN_RESET;
    return MBEDTLS_ERR_NET_SEND_FAILED;
}
